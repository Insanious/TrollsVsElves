#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"
#include "NetworkIDManager.h"
#include "RakPeerInterface.h"
#include "RakPeer.h"

#include <thread>
#include <atomic>
#include <inttypes.h>

#include "GameScreen.h"
#include "ThreadSafeMessageQueue.h"

enum GameMessages
{
    ID_SPAWN_PLAYER             = ID_USER_PACKET_ENUM,
    ID_PLAYER_RMB_REQUEST       = ID_USER_PACKET_ENUM + 1,
    ID_PLAYER_PATH_CORRECTION   = ID_USER_PACKET_ENUM + 2
};

struct SpawnPlayerRequest
{
    RakNet::MessageID packetType;
    Vector3 position;
    PlayerType type;
    RakNet::NetworkID networkId;
    uint64_t ownerGuid;

    void serialize(bool writeToBitstream, RakNet::BitStream *bs)
    {
        bs->Serialize(writeToBitstream, packetType);
        bs->Serialize(writeToBitstream, position.x);
        bs->Serialize(writeToBitstream, position.y);
        bs->Serialize(writeToBitstream, position.z);
        bs->Serialize(writeToBitstream, type);
        bs->Serialize(writeToBitstream, networkId);
        bs->Serialize(writeToBitstream, ownerGuid);
    }

    void print()
    {
        printf("SpawnPlayerRequest::print\n");
        printf("packetType: %d\n", (int)packetType);
        printf("position: %f, %f, %f\n", position.x, position.y, position.z);
        printf("type: %d\n", type);
        printf("networkId: %" PRIu64 "\n", networkId);
        printf("ownerGuid: %" PRIu64 "\n", ownerGuid);
    }
};

struct PlayerRMBRequest
{
    RakNet::MessageID packetType;
    RakNet::NetworkID networkId;
    Vector3 position;

    void serialize(bool writeToBitstream, RakNet::BitStream *bs)
    {
        bs->Serialize(writeToBitstream, packetType);
        bs->Serialize(writeToBitstream, networkId);
        bs->Serialize(writeToBitstream, position.x);
        bs->Serialize(writeToBitstream, position.y);
        bs->Serialize(writeToBitstream, position.z);
    }

    void print()
    {
        printf("PlayerRMBRequest::print\n");
        printf("packetType: %d\n", (int)packetType);
        printf("networkId: %" PRIu64 "\n", networkId);
        printf("position: %f, %f, %f\n", position.x, position.y, position.z);
    }
};

struct PlayerPathCorrection
{
    RakNet::MessageID packetType;
    RakNet::NetworkID networkId;
    size_t nrOfPaths;
    std::vector<Vector3> path;

    void serialize(bool writeToBitstream, RakNet::BitStream *bs)
    {
        bs->Serialize(writeToBitstream, packetType);
        bs->Serialize(writeToBitstream, networkId);
        bs->Serialize(writeToBitstream, nrOfPaths);
        if (!writeToBitstream) path = std::vector<Vector3>(nrOfPaths, { 0 });
        for (int i = 0; i < nrOfPaths; i++)
        {
            bs->Serialize(writeToBitstream, path[i].x);
            bs->Serialize(writeToBitstream, path[i].y);
            bs->Serialize(writeToBitstream, path[i].z);
        }
    }

    void print()
    {
        printf("PlayerPathCorrection::print\n");
        printf("packetType: %d\n", (int)packetType);
        printf("networkId: %" PRIu64 "\n", networkId);
        for (Vector3 position: path)
            printf("position: %f, %f, %f\n", position.x, position.y, position.z);
    }
};

enum NetworkType { NONE = 0, SERVER, CLIENT };

struct NetworkManager
{
    RakNet::RakPeerInterface* rakPeerInterface;
    RakNet::SocketDescriptor socketDescriptor;
    RakNet::NetworkIDManager networkIDManager;
    RakNet::RakNetGUID serverGuid;

    NetworkType networkType = NetworkType::NONE;
    std::atomic<bool> running = true;
    GameScreen* gameScreen = nullptr;
    ThreadSafeMessageQueue messageQueue;

    NetworkManager() = delete;
    NetworkManager(NetworkType networkType, size_t port, GameScreen* gameScreen);
    ~NetworkManager() {}

    bool isClient() { return networkType == NetworkType::CLIENT; }
    unsigned char getPacketIdentifier(RakNet::Packet* packet);
    std::string getPacketName(RakNet::Packet* packet);
    void listen();

    void handleNewIncomingConnection(RakNet::Packet* packet);
    void handleSpawnPlayer(RakNet::Packet* packet);

    void handlePlayerPathCorrection(RakNet::Packet* packet);
    void sendPlayerPathCorrection(Player* player, std::vector<Vector3> path);

    void handlePlayerRMBRequest(RakNet::Packet* packet);
    void sendPlayerRMBRequest(Player* player, Vector3 position);
};

#endif
