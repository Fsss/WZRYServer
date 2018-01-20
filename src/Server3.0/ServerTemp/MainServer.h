#ifndef _MAINSERVER_H_
#define _MAINSERVER_H_

#include "Epoll.h"
#include "Server.h"
#include "MessageProcesser.h"
#include "WZRYMessageProto.pb.h"

#include <set>
#include <map>
#include <string>

namespace WZRY
{

    class MainServer : Server
    {
    public:
        MainServer(int);
        ~MainServer();
        void MainServerLoop();
        virtual void Response(int, CloseRequest) override;
        virtual void Response(int, LoginRequest) override;
        virtual void Response(int, RegisterRequest) override;
        virtual void Response(int, MatchRequest) override;
    private:
        MainServer(const MainServer&) = delete;
        MainServer& operator=(const MainServer&) = delete;
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

        ///std::set<SoloBattle> m_SoloBattleSet;  // ´æÖ¸Õë
    };

}  // namespace WZRY



#endif // _MAINSERVER_H_






















