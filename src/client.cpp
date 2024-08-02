#include "client.hpp"

Client::Client(bufferevent* bev) : bev_(bev), nickname_("Anonymous") {}

bufferevent* Client::getBufferEvent() const
{
    return bev_;
}

void Client::setNickname(const std::string& nickname)
{
    nickname_ = nickname;
}

std::string Client::getNickname() const
{
    return nickname_;
}
