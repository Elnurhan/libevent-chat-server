#include "server.hpp"
#include <event2/event.h>
#include <event2/listener.h>



Server::Server(const char* address, int port)
{
    base_ = event_base_new();
    if (!base_) {
        throw std::runtime_error("Could not initialize libevent!");
    }

    memset(&sin_, 0, sizeof(sin_));
    sin_.sin_family = AF_INET;
    sin_.sin_addr.s_addr = inet_addr(address);
    sin_.sin_port = htons(port);

    listener_ = evconnlistener_new_bind(base_, acceptConnCb, reinterpret_cast<void*>(this),
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
            reinterpret_cast<sockaddr*>(&sin_), sizeof(sin_));

    if (!listener_) {
        event_base_free(base_);
        throw std::runtime_error("Could not create a listener!");
    }
}

Server::~Server()
{
    for (Client* client: clients_) {
        bufferevent_free(client->getBufferEvent());
        delete client;
    }

    evconnlistener_free(listener_);
    event_base_free(base_);
}

void Server::start()
{
    event_base_dispatch(base_);
}

void Server::acceptConnCb(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx)
{
    Server* server = static_cast<Server*>(ctx);
    event_base* base = server->base_;

    bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    Client* client = new Client(bev);
    server->clients_.insert(client);

    bufferevent_setcb(bev, readCb, NULL, eventCb, server);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
    bufferevent_write(bev, "Please enter your nickname: ", 29);
}

void Server::readCb(bufferevent* bev, void* ctx)
{
    Server* server = static_cast<Server*>(ctx);
    Client* sender = nullptr;

    for (Client* client: server->clients_) {
        if (client->getBufferEvent() == bev) {
            sender = client;
            break;
        }
    }

    if (!sender) return;

    char buffer[1024];
    int n;
    while ((n = bufferevent_read(bev, buffer, sizeof(buffer))) > 0) {
        std::string message(buffer, n);
        server->handleMessage(sender, message);
    }
}

void Server::eventCb(bufferevent* bev, short events, void* ctx)
{
    Server* server = static_cast<Server*>(ctx);
    if (events & BEV_EVENT_ERROR) {
        std::cerr << "Error from bufferevent\n";
    }
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        Client* clientToRemove = nullptr;
        for (Client* client: server->clients_) {
            if (client->getBufferEvent() == bev) {
                clientToRemove = client;
                break;
            }
        }

        if (clientToRemove) {
            server->clients_.erase(clientToRemove);
            bufferevent_free(bev);
            delete clientToRemove;
        }
    }
}

void Server::broadcast(const std::string& message, Client* sender)
{
    std::string fullMessage = sender->getNickname() + ":> " + message;
    for (Client* client: clients_) {
        if (client != sender) {
            bufferevent_write(client->getBufferEvent(), fullMessage.c_str(), fullMessage.size());
        }
    }
}

void Server::handleMessage(Client* sender, const std::string& message)
{
    if (sender->getNickname() == "Anonymous") {
        std::string trimmedMessage = message;
        trimmedMessage.erase(trimmedMessage.find_last_not_of(" \n\r\t") + 1);
        sender->setNickname(trimmedMessage);
        std::string welcomeMessage = "Welcome, " + trimmedMessage + "!\n";
        bufferevent_write(sender->getBufferEvent(), welcomeMessage.c_str(), welcomeMessage.size());
    } else {
        broadcast(message, sender);
    }
}






