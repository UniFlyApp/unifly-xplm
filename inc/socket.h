using asio::ip::tcp;

bool recv_message(tcp::socket& socket, unifly::schema::v1::XPlaneMessage* message);
bool send_message(tcp::socket& socket, const unifly::schema::v1::XPLMMessage& message);
