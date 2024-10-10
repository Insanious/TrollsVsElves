#include "Client.h"

Client::Client(int port, GameScreen* gameScreen)
{
    this->type = CLIENT;
    this->gameScreen = gameScreen;

    rakPeerInterface = RakNet::RakPeerInterface::GetInstance();
    socketDescriptor = RakNet::SocketDescriptor(0, "");
    rakPeerInterface->Startup(1, &socketDescriptor, 1);

    printf("Connecting to server...\n");
    RakNet::ConnectionAttemptResult attempt = rakPeerInterface->Connect("127.0.0.1", port, nullptr, 0);
    if (attempt != RakNet::CONNECTION_ATTEMPT_STARTED) {
        printf("Failed to connect to server.\n");
        return;
    }
}

Client::~Client() {}

void Client::listen()
{
    RakNet::Packet* packet = nullptr;
    while (this->running) {
        Task task;
        while (messageQueue.pop(task))
            task();

        while (packet = rakPeerInterface->Receive()) {
            printf("packet: %s\n", getPacketName(packet).c_str());

            switch (getPacketIdentifier(packet)) {
                case ID_CONNECTION_REQUEST_ACCEPTED:    this->serverGUID = packet->guid;        break;
                case ID_DISCONNECTION_NOTIFICATION:     printf("We have been disconnected.\n"); break;
                case ID_CONNECTION_LOST:                printf("Connection lost.\n");           break;
                case ID_SPAWN_PLAYER:                   handleSpawnPlayer(packet);              break;
                case ID_PLAYER_PATH_CORRECTION:         handlePlayerPathCorrection(packet);     break;
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

void Client::handleSpawnPlayer(RakNet::Packet* packet)
{
    RakNet::BitStream bsIn(packet->data, packet->length, false);

    SpawnPlayerRequest spawnPlayer;
    spawnPlayer.serialize(false, &bsIn);

    Player* player = new Player(spawnPlayer.position, spawnPlayer.type);
    player->SetNetworkID(spawnPlayer.networkId);

    gameScreen->messageQueue.push([this, player, spawnPlayer]() {
        this->gameScreen->playerManager->addPlayer(player);
        if (spawnPlayer.isOwner) {
            printf("is owner!\n");
            this->gameScreen->playerManager->clientPlayer = player;
        }
    });
}

void Client::handlePlayerPathCorrection(RakNet::Packet* packet)
{
    RakNet::BitStream bsIn(packet->data, packet->length, false);
    PlayerPathCorrection playerPathCorrection;
    playerPathCorrection.serialize(false, &bsIn);

    gameScreen->messageQueue.push([this, playerPathCorrection]() {
        Player* player = this->gameScreen->playerManager->getPlayerWithNetworkID(playerPathCorrection.networkId);
        if (!player) {
            printf("Unexpected error occured; player with networkId (%u) could not be found\n", playerPathCorrection.networkId);
            return;
        }

        player->correctPath(playerPathCorrection.path);
    });
}

void Client::sendPlayerRMBRequest(Player* player, Vector3 position)
{
    PlayerRMBRequest playerRMB = {
        .packetType = (RakNet::MessageID)ID_PLAYER_RMB_REQUEST,
        .networkId  = player->GetNetworkID(),
        .position   = position
    };

    RakNet::BitStream bsOut;
    playerRMB.serialize(true, &bsOut);
    rakPeerInterface->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, serverGUID, false, 0);
}
