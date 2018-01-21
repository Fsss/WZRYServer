#ifndef _MESSAGEPROCESSER_H_
#define _MESSAGEPROCESSER_H_

#include "EnumClass.h"
#include "WZRYMessageProto.pb.h"

#include <stddef.h>

namespace WZRY
{

    class Server;

    class MessageProcesser
    {
    public:
        MessageProcesser(Server*);
        ~MessageProcesser();
        bool RecvPeek(int);
        void Send(int, Protocol, ::google::protobuf::Message*);
    private:
        MessageProcesser(const MessageProcesser&) = delete;
        MessageProcesser& operator=(const MessageProcesser&) = delete;
        void Send(int, int, void*, int);
        int ReadN(int, void*, size_t);
        int WriteN(int, const void*, size_t);
    private:
        char *m_readBuff;
        char *m_writeBuff;
        Server *m_server;
    };

}  // namespace WZRY





#endif // _MESSAGEPROCESSER_H_


