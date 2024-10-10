#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "GameMessages.h"
#include "BitStream.h"
#include "RakNetTypes.h"
#include "NetworkIDManager.h"
#include "RakPeerInterface.h"

#include <thread>
#include <atomic>
#include <inttypes.h>

#include "GameScreen.h"
#include "ThreadSafeMessageQueue.h"

struct SpawnPlayerRequest
{
    RakNet::MessageID packetType;
    Vector3 position;
    PlayerType type;
    RakNet::NetworkID networkId;
    bool isOwner;

    void serialize(bool writeToBitstream, RakNet::BitStream *bs)
    {
        bs->Serialize(writeToBitstream, packetType);
        bs->Serialize(writeToBitstream, position.x);
        bs->Serialize(writeToBitstream, position.y);
        bs->Serialize(writeToBitstream, position.z);
        bs->Serialize(writeToBitstream, type);
        bs->Serialize(writeToBitstream, networkId);
        bs->Serialize(writeToBitstream, isOwner);
    }

    void print()
    {
        printf("SpawnPlayerRequest::print\n");
        printf("packetType: %d\n", (int)packetType);
        printf("position: %f, %f, %f\n", position.x, position.y, position.z);
        printf("type: %d\n", type);
        printf("networkId: %" PRIu64 "\n", networkId);
        printf("isOwner: %d\n", (int)isOwner);
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
        for (int i = 0; i < nrOfPaths; i++) {
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

enum NetworkType { SERVER = 0, CLIENT };

class NetworkInterface
{
public:
    RakNet::RakPeerInterface* rakPeerInterface;
    RakNet::SocketDescriptor socketDescriptor;
    RakNet::NetworkIDManager networkIDManager;

    NetworkType type;
    std::atomic<bool> running = true;
    ThreadSafeMessageQueue messageQueue;

    NetworkInterface() {}
    ~NetworkInterface() {}

    virtual void listen() {}
    unsigned char getPacketIdentifier(RakNet::Packet* packet);
    std::string getPacketName(RakNet::Packet* packet);
};

#endif
