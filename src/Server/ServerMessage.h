#ifndef _SERVERMESSAGE_H_
#define _SERVERMESSAGE_H_

#include "ServerMessageProto.pb.h"

#include <stddef.h>

namespace WZRY
{

    class Server;

    class ServerMessage
    {
    public:
        ServerMessage(Server&);
        void Receive(int);
        void Send(int, LoginRSPS);
        void Send(int, RegisterRSPS);
        void Send(int, MatchRSPS);
    private:
        ServerMessage(const ServerMessage&) = delete;
        ServerMessage& operator=(const ServerMessage&) = delete;
        void Send(int, int, void*, int);
        int ReadN(int, void*, size_t);
        int WriteN(int, const void*, size_t);
    private:
        Server& m_server;
    };

}  // namespace WZRY


#endif  // _SERVERMESSAGE_H_


