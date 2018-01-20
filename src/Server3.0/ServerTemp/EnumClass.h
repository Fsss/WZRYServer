#ifndef _ENUMCLASS_H_
#define _ENUMCLASS_H_

#define LISTEN_SIZE 100
#define BUFF_SIZE 1024
#define MAX_CONNECT 99

namespace WZRY
{

    enum class Protocol
    {
        CLOSE = 0,
        LOGIN = 1,
        REGISTER = 2,
        MATCH = 3,
        CHAT = 4,
        SKILL = 5
    };

}  // namespace WZRY



#endif // _ENUMCLASS_H_








