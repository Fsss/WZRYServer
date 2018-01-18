#ifndef _MESSAGECENTER_H_
#define _MESSAGECENTER_H_

#include "EnumClass.h"

namespace WZRY
{

    class Server;

    class MessageCenter
    {
    public:
        MessageCenter(Server*);
        void Receive(int);
        template <typename M>
        void Send(int, MessageProtocol, M);
    private:
        MessageCenter(const MessageCenter&) = delete;
        MessageCenter& operator=(const MessageCenter&) = delete;
        void Send(int, int, void*, int);
        int ReadN(int, void*, size_t);
        int WriteN(int, const void*, size_t);
    private:
        Server *m_server;
    };

}  // namespace WZRY


#endif  // _MESSAGECENTER_H_


