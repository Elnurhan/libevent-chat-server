#pragma once

#include "client.hpp"

#include <event2/event.h>
#include <event2/listener.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_set>


class Server
{
public:
    Server(const char* address, int port);
    ~Server();
    void start();

private:
    static void acceptConnCb(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx);
    static void readCb(bufferevent* bev, void* ctx);
    static void eventCb(bufferevent* bev, short events, void* ctx);
    void broadcast(const std::string& message, Client* sender);
    void handleMessage(Client* sender, const std::string& message);

    event_base* base_;
    evconnlistener* listener_;
    sockaddr_in sin_;
    std::unordered_set<Client*> clients_;
};
