#include "utilities.h"
#include <math.h>

using asio::ip::tcp;

// We do all our reading on the socket thread
// And all our writing on the xplane thread
// (with the exception of Open)

bool write_exact(tcp::socket& socket, asio::const_buffer buffer) {
    asio::error_code ec;
    // asio::write will loop internally until the entire buffer is sent
    asio::write(socket, buffer, asio::transfer_all(), ec);
    return !ec;
}

// Sends a length-prefixed Protobuf message
bool send_message(tcp::socket& socket, const unifly::schema::v1::XPLMMessage& message) {
    // Check if the message size can fit in a uint16_t length prefix
    size_t message_size = message.ByteSizeLong();
    if (message_size >= pow(2, 32)) {
		Log("failed to send a message: message is too long");
        return false;
    }
    uint32_t message_length = static_cast<uint32_t>(message_size);

    // Create a buffer large enough for the prefix and the message
    int prefix_length = sizeof(uint32_t);
    std::vector<uint8_t> buffer(prefix_length + message_length);

    // Manually copy the little-endian length prefix to the start of the buffer
    // using a helper from Protobuf to ensure correctness.
    google::protobuf::io::CodedOutputStream::WriteLittleEndian16ToArray(message_length, buffer.data());

    // Serialize the message directly into the buffer after the prefix
    if (!message.SerializeToArray(buffer.data() + prefix_length, message_length)) {
        Log("failed to send a message: failed to serialize message");
        return false;
    }

    // Write the entire buffer to the socket
    return write_exact(socket, asio::buffer(buffer));
}

// Receives a length-prefixed Protobuf message
bool recv_message(tcp::socket& socket, unifly::schema::v1::XPlaneMessage* message) {
    if (!message) {
        Log("recv_message: Message pointer is null");
        return false;
    }

    asio::error_code ec;

    // Read little endian u16 length prefix
    uint8_t prefix_buf[sizeof(uint32_t)]{};
    size_t prefix_read = asio::read(socket, asio::buffer(prefix_buf, sizeof(prefix_buf)), ec);
    if (ec || prefix_read != sizeof(prefix_buf)) {
        Log("recv_message: Failed to read length prefix %s", ec.message().c_str());
        return false;
    }

    uint32_t message_length = 0;
    google::protobuf::io::CodedInputStream::ReadLittleEndian32FromArray(prefix_buf, &message_length);

    if (message_length == 0 || message_length > 65535) {
        Log("recv_message: Invalid message length %i", message_length);
        return false;
    }

    // Read buffer
    std::vector<uint8_t> buffer(message_length);
    size_t body_read = asio::read(socket, asio::buffer(buffer.data(), message_length), ec);
    if (ec || body_read != message_length) {
        Log("recv_message: Failed to read message body %s", ec.message().c_str());
        return false;
    }

    // Parse buffer
    if (!message->ParseFromArray(buffer.data(), message_length)) {
        Log("recv_message: Deserialization failed %s", ec.message().c_str());
        return false;
    }

    return true;
}
