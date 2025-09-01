#include "utilities.h"

using asio::ip::tcp;

bool write_exact(tcp::socket& socket, asio::const_buffer buffer) {
    asio::error_code ec;
    // asio::write will loop internally until the entire buffer is sent
    asio::write(socket, buffer, asio::transfer_all(), ec);
    return !ec;
}

// Sends a length-prefixed Protobuf message
bool send_message(tcp::socket& socket, const unifly::schema::XPLMMessage& message) {
    // Check if the message size can fit in a uint16_t length prefix
    size_t message_size = message.ByteSizeLong();
    if (message_size > 65535) {
        std::cerr << "Message size (" << message_size << ") exceeds uint16_t capacity.\n";
        return false;
    }
    uint16_t message_length = static_cast<uint16_t>(message_size);

    // Create a buffer large enough for the prefix and the message
    int prefix_length = sizeof(uint16_t);
    std::vector<uint8_t> buffer(prefix_length + message_length);

    // Manually copy the little-endian length prefix to the start of the buffer
    // using a helper from Protobuf to ensure correctness.
    google::protobuf::io::CodedOutputStream::WriteLittleEndian16ToArray(message_length, buffer.data());

    // Serialize the message directly into the buffer after the prefix
    if (!message.SerializeToArray(buffer.data() + prefix_length, message_length)) {
        std::cerr << "Failed to serialize message.\n";
        return false;
    }

    // Write the entire buffer to the socket
    return write_exact(socket, asio::buffer(buffer));
}

// Receives a length-prefixed Protobuf message
bool recv_message(tcp::socket& socket, unifly::schema::XPlaneMessage* message) {
    if (!message) {
        Log("recv_message: Message pointer is null");
        return false;
    }

    Log("a");

    asio::error_code ec;

    // Read little endian u16 length prefix
    uint8_t prefix_buf[sizeof(uint16_t)]{};
    size_t prefix_read = asio::read(socket, asio::buffer(prefix_buf, sizeof(prefix_buf)), ec);
    if (ec || prefix_read != sizeof(prefix_buf)) {
        Log("recv_message: Failed to read length prefix %s", ec.message().c_str());
        return false;
    }

    Log("b");

    uint16_t message_length = 0;
    google::protobuf::io::CodedInputStream::ReadLittleEndian16FromArray(prefix_buf, &message_length);

    Log("c %i", message_length);

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

    Log("body_read %i", body_read);

    // print_buffer(buffer.data())

    message->DebugString().c_str();

    Log("dddd %s", message->DebugString().c_str());

    // Parse buffer
    if (!message->ParseFromArray(buffer.data(), message_length)) {
        Log("recv_message: Deserialization failed %s", ec.message().c_str());
        return false;
    }
    Log("e");

    return true;
}
