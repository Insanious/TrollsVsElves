#ifndef CLIENT_H
#define CLIENT_H

#include "NetworkInterface.h"

class Client: public NetworkInterface
{
private:
    RakNet::RakNetGUID serverGUID;
    GameScreen* gameScreen;

public:
    Client() = delete;
    Client(int port, GameScreen* gameScreen);
    ~Client();


    void listen();

    void handleSpawnPlayer(RakNet::Packet* packet);
    void handlePlayerPathCorrection(RakNet::Packet* packet);

    void sendPlayerRMBRequest(Player* player, Vector3 position);
};

#endif
