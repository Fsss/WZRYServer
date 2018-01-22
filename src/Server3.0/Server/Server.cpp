#include "Server.h"
#include "Epoll.h"
#include "EnumClass.h"
#include "MessageProcesser.h"
#include "WZRYMessageProto.pb.h"

#include <thread>
#include <chrono>
#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



namespace WZRY
{

    Server::Server(int port)
        : m_port(port), m_epoll(EPOLL_SIZE), m_messageProcesser(this)
    {
        // socket()
        m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == m_listenfd)
	    {
            printf("Server socket() error!\n");
    		return;
	    }
        // address
	    struct sockaddr_in listen_addr;
	    listen_addr.sin_family = AF_INET;
	    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	    listen_addr.sin_port = htons(port);
        // set SO_REUSEADDR
        int on = 1;
        if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        {
            printf("Set SO_REUSEADDR error!\n");
            return;
        }
        // bind()
	    if (bind(m_listenfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0)
	    {
            printf("Server bind() error!\n");
		    return;
	    }
	    // listen()
	    listen(m_listenfd, MAX_CONNECT);
        // epoll add listenfd
        if (-1 == m_epoll.EpollCtlAdd(m_listenfd))
        {
            printf("Server epoll_ctl error!\n");
            return;
        }
        // init
        for (int i = 0; i < ROOM_SIZE; ++i)
            m_roomSet[0].insert(i);
        // 初始users
        m_users["123"] = "123";
        printf("Server listen in port : %d ...\n", port);
    }

    Server::~Server()
    {
        close(m_listenfd);
        for (std::set<int>::iterator it = m_connectSet.begin(); it != m_connectSet.end(); ++it)
            Disconnect(*it);
    }

    void Server::ServerLoop()
    {
        printf("Server Loop running...\n");
        long lastTime, nowTime;
        lastTime = nowTime = GetCurrentTime();
        while (true)
        {
            int n = m_epoll.EpollWait();
            for (int i = 0; i < n; ++i)
            {
                epoll_event event = m_epoll.GetEvent(i);
                int eventFd = event.data.fd;
                int eventEvents = event.events;
                if ((eventEvents & EPOLLERR) || (eventEvents & EPOLLHUP) || (!(eventEvents & EPOLLIN)))
			    {
				    printf("clientfd : %d error\n", eventFd);
                    Disconnect(eventFd);
				    continue;
			    } else if (eventFd == m_listenfd) {
                    struct sockaddr client_addr;
				    int client_addr_len = sizeof(client_addr);
				    int newClient = accept(m_listenfd, &client_addr, (socklen_t*)&client_addr_len);
                    if (newClient < 0)
                    {
                        printf("new client accept error!\n");
                        continue;
                    }
                    printf("A new client connected. socket = %d\n", newClient);
                    m_epoll.EpollCtlAdd(newClient);
                    m_connectSet.insert(newClient);
                } else {
                    /// 从ET的epoll中读出所有的消息
                    while (m_messageProcesser.RecvPeek(eventFd));
                }
            }
            // 发送帧结束包
            nowTime = GetCurrentTime();
            if (nowTime - lastTime > FRAME_INTERVAL)
            {
                lastTime = nowTime;
                SendFrameOver();
            }
        }
    }

    long Server::GetCurrentTime()
    {
        struct timeval tv;    
        gettimeofday(&tv,NULL);    
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    void Server::Disconnect(int sock)
    {
        printf("Client : %d close the connect.\n", sock);
        m_epoll.EpollCtlDel(sock);
        close(sock);
        CancelSoloBattle(sock);
        m_connectSet.erase(sock);
    }

    void Server::UserLogout(int sock)
    {
        printf("Client : %d logout.\n", sock);
        CancelSoloBattle(sock);
        m_loginUsers.erase(sock);
    }

    void Server::RequestSoloBattle(int sock)
    {
        printf("Client : %d require solo battle match!\n", sock);
        m_requestSoloBattleSet.insert(sock);
        if (m_requestSoloBattleSet.size() >= 2)
        {
            if (0 == m_roomSet[0].size())
            {
                printf("No spare room!\n");
                return;
            }
            printf("Begin a solo battle!\n");
            int roomId = *(m_roomSet[0].begin());
            m_roomSet[0].erase(m_roomSet[0].begin());
            m_roomSet[1].insert(roomId);

            int player1, player2;
            player1 = *(m_requestSoloBattleSet.begin());
            m_requestSoloBattleSet.erase(m_requestSoloBattleSet.begin());
            player2 = *(m_requestSoloBattleSet.begin());
            m_requestSoloBattleSet.erase(m_requestSoloBattleSet.begin());

            m_room[roomId].push_back(player1);
            m_room[roomId].push_back(player2);

            MatchResponse match;
            match.set_issuccess(true);
            match.set_matchmessage("匹配成功");

            for (unsigned int i = 0; i < m_room[roomId].size(); ++i)
            {
                match.set_id(i + 1);
                m_sockRoomId[m_room[roomId][i]] = roomId;
                m_messageProcesser.Send(m_room[roomId][i], Protocol::MATCH, &match);
            }
        }
    }

    void Server::CancelSoloBattle(int sock)
    {
        if (m_requestSoloBattleSet.count(sock))
        {
            printf("Client : %d cancel solo battle match!\n", sock);
            m_requestSoloBattleSet.erase(sock);
        }
    }

    void Server::SendFrameOver()
    {
        FrameOver frameOver;
        for (auto roomId : m_roomSet[1])
            Response(roomId, frameOver);
    }


    /// lobby
    void Server::Response(int sock, CloseRequest login)
    {
        Disconnect(sock);
    }

    void Server::Response(int sock, LoginRequest login)
    {
        printf("Receive client : %d a new Login message!\n", sock);
        printf("LoginRequest:  username = %s   passwd = %s\n\n", login.username().c_str(), login.passwd().c_str());
        if (login.islogin())
        {
            LoginResponse response;
            if (m_users.end() != m_users.find(login.username()))
            {
                if (login.passwd() == m_users[login.username()])
                {
                    response.set_issuccess(true);
                    response.set_message("登录成功");
                    m_loginUsers[sock] = login.username();
                }
                else
                {
                    response.set_issuccess(false);
                    response.set_message("密码错误");
                }
            }
            else
            {
                response.set_issuccess(false);
                response.set_message("账号不存在");
            }
            m_messageProcesser.Send(sock, Protocol::LOGIN, &response);
        }
        else {
            UserLogout(sock);
        }
    }

    void Server::Response(int sock, RegisterRequest rgst)
    {
        printf("Receive client : %d a new Register message!\n", sock);
        printf("RegisterRequest:  newusername = %s   newpasswd = %s\n\n", rgst.newusername().c_str(), rgst.newpasswd().c_str());
        RegisterResponse response;
        if (m_users.end() != m_users.find(rgst.newusername()))
        {
            response.set_issuccess(false);
            response.set_message("账号已存在");
        }
        else
        {
            m_users[rgst.newusername()] = rgst.newpasswd();
            response.set_issuccess(true);
            response.set_message("注册成功");
        }
        m_messageProcesser.Send(sock, Protocol::REGISTER, &response);
    }

    void Server::Response(int sock, MatchRequest match)
    {
        printf("Receive client : %d a new Match message!\n", sock);
        printf("MatchRequest:  ismatch = %d [true|false]   matchtype = %d\n\n", match.ismatch(), match.matchtype());
        if (match.ismatch())
        {
            if (2 == match.matchtype())
            {
                RequestSoloBattle(sock);
            }
            else {
                printf("Request invalid battle!\n");
            }
        }
        else
        {
            if (2 == match.matchtype())
            {
                CancelSoloBattle(sock);
            }
            else {
                printf("Cancel invalid battle!\n");
            }
        }
    }

    /// game
    void Server::Response(int sock, MoveData move)
    {
        int roomId = m_sockRoomId[sock];
        for (unsigned int i = 0; i < m_room[roomId].size(); ++i)
            m_messageProcesser.Send(m_room[roomId][i], Protocol::MOVE, &move);
    }

    void Server::Response(int sock, SkillData skill)
    {
        int roomId = m_sockRoomId[sock];
        for (unsigned int i = 0; i < m_room[roomId].size(); ++i)
            m_messageProcesser.Send(m_room[roomId][i], Protocol::SKILL, &skill);
    }

    void Server::Response(int sock, ChatData chat)
    {
        int roomId = m_sockRoomId[sock];
        for (unsigned int i = 0; i < m_room[roomId].size(); ++i)
            m_messageProcesser.Send(m_room[roomId][i], Protocol::CHAT, &chat);
    }

    void Server::Response(int roomId, FrameOver frameover)
    {
        for (unsigned int i = 0; i < m_room[roomId].size(); ++i)
            m_messageProcesser.Send(m_room[roomId][i], Protocol::FRAMEOVER, &frameover);
    }

    void Server::Response(int sock, GameOver gameover)
    {
        int roomId = m_sockRoomId[sock];
        for (std::vector<int>::iterator it = m_room[roomId].begin(); it != m_room[roomId].end(); ++it)
        if (*it == sock)
        {
            m_room[roomId].erase(it);
            break;
        }
        m_sockRoomId.erase(sock);
        if (0 == m_room[roomId].size()) 
        {
            m_roomSet[1].erase(roomId);
            m_roomSet[0].erase(roomId);
        }
    }


}  // namespace WZRY




