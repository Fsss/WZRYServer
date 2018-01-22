#ifndef _ENUMCLASS_H_
#define _ENUMCLASS_H_

#define EPOLL_SIZE 101
#define BUFF_SIZE 1024
#define MAX_CONNECT 100
#define ROOM_SIZE 100
#define SOLO_BATTLE_SIZE 2
#define FRAME_INTERVAL 20
#define EPOLL_WAIT_TIMEOUT 3

#define MIN_PROTOCOL 0
#define MAX_PROTOCOL 8

namespace WZRY
{

    enum class Protocol
    {
        CLOSE = 0,
        LOGIN = 1,
        REGISTER = 2,
        MATCH = 3,

        MOVE = 4,
        SKILL = 5,
        CHAT = 6,
        FRAMEOVER = 7,
        GAMEOVER = 8
    };

}  // namespace WZRY



#endif // _ENUMCLASS_H_








