using asio::ip::tcp;

bool recv_message(tcp::socket& socket, unifly::schema::XPlaneMessage* message);
bool send_message(tcp::socket& socket, const unifly::schema::XPLMMessage& message);
