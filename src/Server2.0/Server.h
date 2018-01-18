#ifndef _SERVER_H_
#define _SERVER_H_

#include <set>
#include <map>
#include <string>

#include "Epoll.h"
#include "SoloBattle.h"
#include "ServerMessage.h"
#include "ServerMessageProto.pb.h"


namespace WZRY
{

    class Server
    {
    public:
        Server(int);
        ~Server();
        void ServerLoop();
        void ResponseMessage(int, LoginMSG);
        void ResponseMessage(int, RegisterMSG);
        void ResponseMessage(int, MatchMSG);
        void DeleteClient(int);
    private:
        Server(const Server&) = delete;
        Server& operator=(const Server&) = delete;
        void AddSolo(int);
        void DeleteSolo(int);
    private:
        int m_port;
        int m_listenfd;
        Epoll m_epoll;

        std::set<int> m_connetcSet;
        std::set<int> m_loginSet;
        std::set<int> m_soloSet;
        std::map<std::string, std::string> m_users;
        std::map<int, std::string> m_loginUsers;
        ServerMessage m_serverMessage;

        std::set<SoloBattle> m_soloBattle;
    };

}


#endif // _SERVER_H_



