#include "MessageIdentifiers.h"

enum GameMessages
{
    ID_SPAWN_PLAYER             = ID_USER_PACKET_ENUM,
    ID_PLAYER_RMB_REQUEST       = ID_USER_PACKET_ENUM + 1,
    ID_PLAYER_PATH_CORRECTION   = ID_USER_PACKET_ENUM + 2
};