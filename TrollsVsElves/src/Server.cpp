#include "Server.h"

Server::Server(int port, int maxClients, GameScreen* gameScreen)
{
    this->type = SERVER;
    this->gameScreen = gameScreen;

    rakPeerInterface = RakNet::RakPeerInterface::GetInstance();
    socketDescriptor = RakNet::SocketDescriptor(port, 0);

    rakPeerInterface->Startup(maxClients, &socketDescriptor, 1, 10);
    rakPeerInterface->SetMaximumIncomingConnections(maxClients);
    printf("Server started, waiting for connections...\n");
}

Server::~Server() {}

void Server::listen()
{

    RakNet::Packet* packet = nullptr;
    while (this->running) {
        Task task;
        while (messageQueue.pop(task))
            task();

        while (packet = rakPeerInterface->Receive()) {
            printf("packet: %s\n", getPacketName(packet).c_str());

            switch (getPacketIdentifier(packet)) {
                case ID_NEW_INCOMING_CONNECTION:    handleNewIncomingConnection(packet);        break;
                case ID_DISCONNECTION_NOTIFICATION: printf("A client has disconnected.\n");     break;
                case ID_CONNECTION_LOST:            printf("A client lost the connection.\n");  break;
                case ID_SPAWN_PLAYER:               printf("ID_SPAWN_PLAYER.\n");               break;
                case ID_PLAYER_RMB_REQUEST:         handlePlayerRMBRequest(packet);             break;
                default: printf("Received message with identifier %d\n", packet->data[0]);      break;
            }
            rakPeerInterface->DeallocatePacket(packet);
            packet = nullptr;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    rakPeerInterface->Shutdown(300);
    RakNet::RakPeerInterface::DestroyInstance(rakPeerInterface);
}

void Server::handleNewIncomingConnection(RakNet::Packet* packet)
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
        .isOwner    = true
    };

    // broadcast new players to all clients
    RakNet::BitStream bsOut;
    spawnPlayer.serialize(true, &bsOut);
    rakPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true, 0);

    // send all current players to the new client
    for (Player* player: gameScreen->playerManager->players)
    {
        bsOut.Reset();
        spawnPlayer = {
            .packetType = packetType,
            .position = player->getPosition(),
            .type = player->playerType,
            .networkId = player->GetNetworkID(),
            .isOwner = false
        };
        spawnPlayer.serialize(true, &bsOut);
        rakPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false, 0);
    }

    // finally, add the new player to the game
    this->gameScreen->messageQueue.push([this, newPlayer]() {
        this->gameScreen->playerManager->addPlayer(newPlayer);
    });
}

void Server::handlePlayerRMBRequest(RakNet::Packet* packet)
{
    RakNet::BitStream bsIn(packet->data, packet->length, false);

    // IMPROVEMENT: make sure this request is coming from a client that owns the player
    PlayerRMBRequest playerRMB;
    playerRMB.serialize(false, &bsIn);
    // std::this_thread::sleep_for(std::chrono::milliseconds(400)); // artificial latency
    this->gameScreen->messageQueue.push([this, playerRMB]() {
        Player* player = this->gameScreen->playerManager->getPlayerWithNetworkID(playerRMB.networkId);
        if (!player) {
            printf("Unexpected error occured; player with networkId (%u) could not be found\n", playerRMB.networkId);
            return;
        }

        // update server state and broadcast path to all clients
        std::vector<Vector3> path = this->gameScreen->playerManager->pathfindPlayerToPosition(player, playerRMB.position);
        this->messageQueue.push([this, player, path]() { this->sendPlayerPathCorrection(player, path); });
    });
}

void Server::sendPlayerPathCorrection(Player* player, std::vector<Vector3> path)
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