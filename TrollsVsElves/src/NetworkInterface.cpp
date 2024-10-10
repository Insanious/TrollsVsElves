#include "NetworkInterface.h"

unsigned char NetworkInterface::getPacketIdentifier(RakNet::Packet* packet)
{
    if (packet == nullptr)
        return 255;

    return (unsigned char)packet->data[0] == ID_TIMESTAMP
        ? (unsigned char)packet->data[sizeof(unsigned char) + sizeof(unsigned long)]
        : (unsigned char)packet->data[0];
}

std::string NetworkInterface::getPacketName(RakNet::Packet* packet)
{
    switch (getPacketIdentifier(packet))
    {
        case ID_NEW_INCOMING_CONNECTION:    return "ID_NEW_INCOMING_CONNECTION";
        case ID_DISCONNECTION_NOTIFICATION: return "ID_DISCONNECTION_NOTIFICATION";
        case ID_CONNECTION_LOST:            return "ID_CONNECTION_LOST";
        case ID_SPAWN_PLAYER:               return "ID_SPAWN_PLAYER";
        case ID_PLAYER_RMB_REQUEST:         return "ID_PLAYER_RMB_REQUEST";
        case ID_PLAYER_PATH_CORRECTION:     return "ID_PLAYER_PATH_CORRECTION";
        default:                            return "unknown";
    }
}