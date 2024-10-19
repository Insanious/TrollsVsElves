#include "NetworkManager.h"

NetworkManager::NetworkManager(NetworkType networkType, size_t port, GameScreen* gameScreen)
{
    this->networkType = networkType;
    this->gameScreen = gameScreen;

    rakPeerInterface = RakNet::RakPeerInterface::GetInstance();

    switch (networkType)
    {
        case NONE:
            running = false;
            break;

        case SERVER:
            socketDescriptor = RakNet::SocketDescriptor(port, 0);
            rakPeerInterface->Startup(constants::MAX_PLAYERS, &socketDescriptor, 1, 10);
            rakPeerInterface->SetMaximumIncomingConnections(constants::MAX_PLAYERS);
            this->serverGuid = rakPeerInterface->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS);
            break;

        case CLIENT:
            socketDescriptor = RakNet::SocketDescriptor(0, "");
            rakPeerInterface->Startup(1, &socketDescriptor, 1);

            printf("Connecting to server...\n");
            RakNet::ConnectionAttemptResult attempt = rakPeerInterface->Connect("127.0.0.1", port, nullptr, 0);
            if (attempt != RakNet::CONNECTION_ATTEMPT_STARTED)
            {
                printf("Failed to connect to server.\n");
                running = false;
            }
            break;
    }
}

unsigned char NetworkManager::getPacketIdentifier(RakNet::Packet* packet)
{
    if (packet == nullptr)
        return 255;

    if ((unsigned char)packet->data[0] == ID_TIMESTAMP)
        return (unsigned char)packet->data[sizeof(unsigned char) + sizeof(unsigned long)];

    return (unsigned char)packet->data[0];
}

std::string NetworkManager::getPacketName(RakNet::Packet* packet)
{
    switch (getPacketIdentifier(packet))
    {
        case ID_NEW_INCOMING_CONNECTION:    return "ID_NEW_INCOMING_CONNECTION";
        case ID_DISCONNECTION_NOTIFICATION: return "ID_DISCONNECTION_NOTIFICATION";
        case ID_CONNECTION_LOST:            return "ID_CONNECTION_LOST";
        case ID_SPAWN_PLAYER:               return "ID_SPAWN_PLAYER";
        case ID_PLAYER_RMB_REQUEST:         return "ID_PLAYER_RMB_REQUEST";
        case ID_PLAYER_PATH_CORRECTION:     return "ID_PLAYER_PATH_CORRECTION";
        default:                            return "UNKNOWN PACKET IDENTIFIER";
    }
}

void NetworkManager::listen()
{
    RakNet::Packet* packet = nullptr;
    unsigned char packetId;
    Task task;

    while (running)
    {
        while (messageQueue.pop(task))
            task();

        while (packet = rakPeerInterface->Receive())
        {
            printf("packet: %s\n", getPacketName(packet).c_str());
            packetId = getPacketIdentifier(packet);

            switch (networkType)
            {
                case SERVER:
                {
                    switch (packetId)
                    {
                        case ID_NEW_INCOMING_CONNECTION:    handleNewIncomingConnection(packet);        break;
                        case ID_DISCONNECTION_NOTIFICATION: printf("A client has disconnected.\n");     break;
                        case ID_CONNECTION_LOST:            printf("A client lost the connection.\n");  break;
                        case ID_SPAWN_PLAYER:               printf("ID_SPAWN_PLAYER.\n");               break;
                        case ID_PLAYER_RMB_REQUEST:         handlePlayerRMBRequest(packet);             break;
                        default: printf("Received message with identifier %d\n", packet->data[0]);      break;
                    }
                }
                case CLIENT:
                {
                    switch (packetId)
                    {
                        case ID_CONNECTION_REQUEST_ACCEPTED:    serverGuid = packet->guid;              break;
                        case ID_DISCONNECTION_NOTIFICATION:     printf("We have been disconnected.\n"); break;
                        case ID_CONNECTION_LOST:                printf("Connection lost.\n");           break;
                        case ID_SPAWN_PLAYER:                   handleSpawnPlayer(packet);              break;
                        case ID_PLAYER_PATH_CORRECTION:         handlePlayerPathCorrection(packet);     break;
                        default: printf("Received message with identifier %d\n", packet->data[0]);      break;
                    }
                }
            }

            rakPeerInterface->DeallocatePacket(packet);
            packet = nullptr;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    rakPeerInterface->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(rakPeerInterface);
}

// Server
void NetworkManager::handleNewIncomingConnection(RakNet::Packet* packet)
{
    PlayerType type = PlayerType::PLAYER_ELF;
    Vector3 position = { 44, 2, 60 };
    Player* newPlayer = new Player(position, type);
    newPlayer->SetNetworkIDManager(&networkIDManager);

    RakNet::MessageID packetType = (RakNet::MessageID)ID_SPAWN_PLAYER;
    SpawnPlayerRequest spawnPlayer = {
        .packetType = packetType,
        .position   = position,
        .type       = type,
        .networkId  = newPlayer->GetNetworkID(),
        .ownerGuid  = packet->guid.g
    };

    // broadcast new player to all clients
    RakNet::BitStream bsOut;
    spawnPlayer.serialize(true, &bsOut);
    rakPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, 0);

    // send all current players to the new client
    for (Player* player: gameScreen->playerManager->players) // WARNING: subject to race-condition
    {
        bsOut.Reset();
        spawnPlayer = {
            .packetType = packetType,
            .position   = player->getPosition(),
            .type       = player->playerType,
            .networkId  = player->GetNetworkID(),
            .ownerGuid  = 0,
        };
        spawnPlayer.serialize(true, &bsOut);
        rakPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false, 0);
    }

    // finally, add the new player to the game
    this->gameScreen->messageQueue.push([this, newPlayer]() {
        this->gameScreen->playerManager->addPlayer(newPlayer);
    });
}

// Client
void NetworkManager::handleSpawnPlayer(RakNet::Packet* packet)
{
    RakNet::BitStream bsIn(packet->data, packet->length, false);

    SpawnPlayerRequest spawnPlayer;
    spawnPlayer.serialize(false, &bsIn);

    Player* player = new Player(spawnPlayer.position, spawnPlayer.type);
    player->SetNetworkID(spawnPlayer.networkId);
    RakNet::RakNetGUID guid = rakPeerInterface->GetGuidFromSystemAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS);
    bool isOwner = (spawnPlayer.ownerGuid != 0) && (guid.g == spawnPlayer.ownerGuid);

    gameScreen->messageQueue.push([this, player, spawnPlayer, isOwner]() {
        this->gameScreen->playerManager->addPlayer(player);
        if (isOwner)
            this->gameScreen->playerManager->clientPlayer = player;
    });
}

// Client
void NetworkManager::handlePlayerPathCorrection(RakNet::Packet* packet)
{
    RakNet::BitStream bsIn(packet->data, packet->length, false);
    PlayerPathCorrection playerPathCorrection;
    playerPathCorrection.serialize(false, &bsIn);

    gameScreen->messageQueue.push([this, playerPathCorrection]() {
        Player* player = this->gameScreen->playerManager->getPlayerWithNetworkID(playerPathCorrection.networkId);
        if (!player)
        {
            printf("Unexpected error occured; player with networkId (%u) could not be found\n", playerPathCorrection.networkId);
            return;
        }

        player->correctPath(playerPathCorrection.path);
    });
}

// Server
void NetworkManager::sendPlayerPathCorrection(Player* player, std::vector<Vector3> path)
{
    PlayerPathCorrection playerPathCorrection = {
        .packetType = (RakNet::MessageID)ID_PLAYER_PATH_CORRECTION,
        .networkId  = player->GetNetworkID(),
        .nrOfPaths  = path.size(),
        .path       = path
    };

    RakNet::BitStream bsOut;
    playerPathCorrection.serialize(true, &bsOut);
    rakPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, 0);
}

// Server
void NetworkManager::handlePlayerRMBRequest(RakNet::Packet* packet)
{
    RakNet::BitStream bsIn(packet->data, packet->length, false);

    // IMPROVEMENT: make sure this request is coming from a client that owns the player
    PlayerRMBRequest playerRMB;
    playerRMB.serialize(false, &bsIn);
    // std::this_thread::sleep_for(std::chrono::milliseconds(400)); // artificial latency
    this->gameScreen->messageQueue.push([this, playerRMB]() {
        Player* player = this->gameScreen->playerManager->getPlayerWithNetworkID(playerRMB.networkId);
        if (!player)
        {
            printf("Unexpected error occured; player with networkId (%u) could not be found\n", playerRMB.networkId);
            return;
        }

        // update server state and broadcast path to all clients
        std::vector<Vector3> path = this->gameScreen->playerManager->pathfindPlayerToPosition(player, playerRMB.position);
        this->messageQueue.push([this, player, path]() { this->sendPlayerPathCorrection(player, path); });
    });
}

// Client
void NetworkManager::sendPlayerRMBRequest(Player* player, Vector3 position)
{
    PlayerRMBRequest playerRMB = {
        .packetType = (RakNet::MessageID)ID_PLAYER_RMB_REQUEST,
        .networkId  = player->GetNetworkID(),
        .position   = position
    };

    RakNet::BitStream bsOut;
    playerRMB.serialize(true, &bsOut);
    rakPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverGuid, false, 0);
}