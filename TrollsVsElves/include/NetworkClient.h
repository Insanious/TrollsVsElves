#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include "Server.h"
#include "Client.h"

#include <cassert>

class NetworkClient
{
private:
    NetworkClient() {}

public:
    Server* server = nullptr;
    Client* client = nullptr;

    NetworkClient(NetworkClient const&) = delete;
    void operator=(NetworkClient const&) = delete;
    ~NetworkClient() {}

    static NetworkClient& get()
    {
        static NetworkClient instance;
        return instance;
    }

    void setNetworkInterface(NetworkInterface* interface)
    {
        if (interface->type == SERVER)  server = (Server*)interface;
        else                            client = (Client*)interface;
    }

    bool isServer() { return server != nullptr; }
    bool isClient() { return client != nullptr; }

    Client* getClient() { assert(client != nullptr); return client; }
};

#endif
