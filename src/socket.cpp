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

void print_buffer(const asio::const_buffer& buffer) {
    // Get the pointer to the data
    const unsigned char* data = static_cast<const unsigned char*>(buffer.data());
    // Get the size of the buffer
    std::size_t size = buffer.size();

    for (std::size_t i = 0; i < size; ++i) {
        // Print each byte in hex with leading zeros and a space
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(data[i]) << ' ';
    }
    std::cout << std::dec << std::endl; // Reset to decimal output
}

// Receives a length-prefixed Protobuf message
bool recv_message(tcp::socket& socket, unifly::schema::XPlaneMessage* message) {
    if (!message) {
        std::cerr << "Message pointer is null!\n";
        return false;
    }

    asio::error_code ec;

    // Read little endian u16 length prefix
    uint8_t prefix_buf[sizeof(uint16_t)]{};
    size_t prefix_read = asio::read(socket, asio::buffer(prefix_buf, sizeof(prefix_buf)), ec);
    if (ec || prefix_read != sizeof(prefix_buf)) {
        std::cerr << "Failed to read length prefix: " << ec.message() << "\n";
        return false;
    }

    uint16_t message_length = 0;
    google::protobuf::io::CodedInputStream::ReadLittleEndian16FromArray(prefix_buf, &message_length);

    if (message_length == 0 || message_length > 65535) {
        std::cerr << "Invalid message length: " << message_length << "\n";
        return false;
    }

    // Read buffer
    std::vector<uint8_t> buffer(message_length);
    size_t body_read = asio::read(socket, asio::buffer(buffer.data(), message_length), ec);
    if (ec || body_read != message_length) {
        std::cerr << "Failed to read message body: " << ec.message() << "\n";
        return false;
    }

    // Parse buffer
    if (!message->ParseFromArray(buffer.data(), message_length)) {
        std::cerr << "ParseFromArray failed — payload corrupted or schema mismatch.\n";
        return false;
    }

    return true;
}
