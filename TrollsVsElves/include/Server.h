#ifndef SERVER_H
#define SERVER_H

#include "NetworkInterface.h"

class Server: public NetworkInterface
{
private:
    GameScreen* gameScreen;

public:
    Server() = delete;
    Server(int port, int maxClients, GameScreen* gameScreen);
    ~Server();

    void listen();

    void handleNewIncomingConnection(RakNet::Packet* packet);
    void handlePlayerRMBRequest(RakNet::Packet* packet);

    void sendPlayerPathCorrection(Player* player, std::vector<Vector3> path);
};

#endif
