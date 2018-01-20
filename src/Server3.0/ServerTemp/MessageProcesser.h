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
        template <typename M>
        void Send(int sock, Protocol protocol, M response)
        {
            size_t sz = response.ByteSizeLong();
            void *buff = malloc(sz);
            response.SerializeToArray(buff, sz);
            std::cout << "the message size = " << sz << "\n";
            Send(sock, static_cast<int>(protocol), buff, sz);
            free(buff);
        }
    private:
        MessageProcesser(const MessageProcesser&) = delete;
        MessageProcesser& operator=(const MessageProcesser&) = delete;
        void Send(int, int, void*, int);
        int ReadN(int, void*, size_t);
        int WriteN(int, const void*, size_t);
    private:
        char *m_buff;
        Server *m_server;
    };

}  // namespace WZRY





#endif // _MESSAGEPROCESSER_H_


