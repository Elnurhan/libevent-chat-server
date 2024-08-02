#pragma once

#include <event2/bufferevent.h>
#include <string>

class Client
{
public:
    Client(bufferevent* bev);

    bufferevent* getBufferEvent() const;
    void setNickname(const std::string& nickname);
    std::string getNickname() const;

private:
    bufferevent* bev_;
    std::string nickname_;
};
