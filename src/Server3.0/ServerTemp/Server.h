#ifndef _SERVER_H_
#define _SERVER_H_

#include "WZRYMessageProto.pb.h"

namespace WZRY
{

    class Server
    {
    public:
        virtual void Response(int, LoginRequest);
        virtual void Response(int, RegisterRequest);
        virtual void Response(int, MatchRequest);
        virtual void Response(int, ChatRequest);
        virtual void Response(int, SkillRequest);
        virtual void Response(int, CloseRequest);
    };

}  // namespace WZRY


#endif // _BASESERVER_H_




