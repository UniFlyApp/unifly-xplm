#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "message.pb.h"

#include <google/protobuf/io/coded_stream.h>

// Defined seperately in the Rust code
#define XPLM_PORT 9925

bool send_message(int socket, unifly::schema::XPLMMessage message)
{
  google::protobuf::uint16 message_length = message.ByteSizeLong();
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

bool recv_message(int socket, unifly::schema::XPLMMessage *message)
{
  google::protobuf::uint16 message_length;
  int prefix_length = sizeof(message_length);
  google::protobuf::uint8 prefix[prefix_length];

  if (prefix_length != read(socket, prefix, prefix_length)) {
    return false;
  }
  google::protobuf::io::CodedInputStream::ReadLittleEndian16FromArray(prefix,
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
