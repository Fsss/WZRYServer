#ifndef _SERVER_H_
#define _SERVER_H_

#include "Epoll.h"
#include "MessageProcesser.h"
#include "WZRYMessageProto.pb.h"

#include <set>
#include <map>
#include <string>

namespace WZRY
{

    class Server
    {
    public:
        Server(int);
        ~Server();
        void ServerLoop();
        void Response(int, CloseRequest);
        void Response(int, LoginRequest);
        void Response(int, RegisterRequest);
        void Response(int, MatchRequest);

        void Response(int, MoveData);
        void Response(int, SkillData);
        void Response(int, ChatData);
        void Response(int, FrameOver);
        void Response(int, GameOver);
    private:
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        void DeleteConnect(int);
        void UserLogout(int);
        void RequestSoloBattle(int);
        void CancelSoloBattle(int);
    private:
        int m_port;
        int m_listenfd;
        Epoll m_epoll;
        MessageProcesser m_messageProcesser;
        std::set<int> m_connectSet;
        std::set<int> m_requestSoloBattleSet;
        std::map<int, std::string> m_loginUsers;
        std::map<std::string, std::string> m_users;
    };

}  // namespace WZRY



#endif // _SERVER_H_






















