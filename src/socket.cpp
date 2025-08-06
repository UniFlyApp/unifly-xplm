#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "message.pb.h"

// Defined seperately in the Rust code
#define XPLM_PORT 9925


bool send_message(int socket, unifly::schema::XPLMMessage message)
{
  google::protobuf::uint32 message_length = message.ByteSize();
  int prefix_length = sizeof(message_length);
  int buffer_length = prefix_length + message_length;
  google::protobuf::uint8 buffer[buffer_length];

  google::protobuf::io::ArrayOutputStream array_output(buffer, buffer_length);
  google::protobuf::io::CodedOutputStream coded_output(&array_output);

  coded_output.WriteLittleEndian32(message_length);
  message.SerializeToCodedStream(&coded_output);

  int sent_bytes = write(socket, buffer, buffer_length);
  if (sent_bytes != buffer_length) {
    return false;
  }

  return true;
}

bool recv_message(int socket, my_protobuf::Message *message)
{
  google::protobuf::uint32 message_length;
  int prefix_length = sizeof(message_length);
  google::protobuf::uint8 prefix[prefix_length];

  if (prefix_length != read(socket, prefix, prefix_length)) {
    return false;
  }
  google::protobuf::io::CodedInputStream::ReadLittleEndian32FromArray(prefix,
      &message_length);

  google::protobuf::uint8 buffer[message_length];
  if (message_length != read(socket, buffer, message_length)) {
    return false;
  }
  google::protobuf::io::ArrayInputStream array_input(buffer, message_length);
  google::protobuf::io::CodedInputStream coded_input(&array_input);

  if (!message->ParseFromCodedStream(&coded_input)) {
    return false;
  }

  return true;
}

bool read_exact(int fd, void* buffer, size_t size) {
    char* ptr = static_cast<char*>(buffer);
    size_t total_read = 0;
    while (total_read < size) {
        ssize_t n = recv(fd, ptr + total_read, size - total_read, 0);
        if (n <= 0) return false;
        total_read += n;
    }
    return true;
}

bool read_message(int fd, google::protobuf::Message &msg) {
    uint32_t len_be;
    if (!read_exact(fd, &len_be, 4)) {
        std::cerr << "Failed to read message length\n";
        return false;
    }

    // Convert from big-endian to host byte order
    uint32_t len = ntohl(len_be);

    if (len == 0 || len > 10 * 1024 * 1024) {  // sanity check
        std::cerr << "Invalid message length: " << len << "\n";
        return false;
    }

    std::vector<char> buffer(len);
    if (!read_exact(fd, buffer.data(), len)) {
        std::cerr << "Failed to read message body\n";
        return false;
    }

    return msg.ParseFromArray(buffer.data(), len);
}

void worker_thread(const std::string &host) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(XPLM_PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&server, sizeof(server));
    listen(server, 5);



    // inet_pton(AF_INET, host.c_str(), &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return;
    }

    std::cout << "Connected to " << host << ":" << XPLM_PORT << "\n";

    while (true) {
        YourMessage msg;  // Replace with your actual message type
        if (!read_message(sock, msg)) {
            std::cerr << "Connection closed or message failed.\n";
            break;
        }

        std::cout << "Received message:\n" << msg.DebugString() << "\n";

        // Optionally: process or respond
    }

    close(sock);
}
