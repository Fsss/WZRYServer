#include "Epoll.h"
#include "EnumClass.h"
#include "MainServer.h"
#include "MessageProcesser.h"
#include "WZRYMessageProto.pb.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



namespace WZRY
{

    MainServer::MainServer(int port)
        : m_port(port), m_epoll(LISTEN_SIZE), m_messageProcesser(this)
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
        // bind
	    if (bind(m_listenfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0)
	    {
            printf("Server bind() error!\n");
		    return;
	    }
	    // listen
	    listen(m_listenfd, MAX_CONNECT);

        if (-1 == m_epoll.EpollCtlAdd(m_listenfd))
        {
            printf("Server epoll_ctl error!\n");
            return;
        }
        // 初始users
        m_users["123"] = "123";
        printf("Server listen in port : %d ...\n", port);
    }

    MainServer::~MainServer()
    {
        close(m_listenfd);
    }

    void MainServer::MainServerLoop()
    {
        printf("Server Loop running...\n");
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
                    DeleteConnect(eventFd);
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
                    /// 一直读直到没消息或消息不完整
                    while (m_messageProcesser.RecvPeek(eventFd));
                }
            }
        }
    }

    void MainServer::DeleteConnect(int sock)
    {
        printf("Client : %d close the connect.\n", sock);
        m_epoll.EpollCtlDel(sock);
        close(sock);
        CancelSoloBattle(sock);
        m_connectSet.erase(sock);
    }

    void MainServer::UserLogout(int sock)
    {
        printf("Client : %d logout.\n", sock);
        CancelSoloBattle(sock);
        m_loginUsers.erase(sock);
    }

    void MainServer::RequestSoloBattle(int sock)
    {
        printf("Client : %d require solo battle match!\n", sock);
        m_requestSoloBattleSet.insert(sock);
        if (m_requestSoloBattleSet.size() >= 2)
        {
            printf("Begin a solo battle!\n");
            /*

            */
        }
    }

    void MainServer::CancelSoloBattle(int sock)
    {
        if (m_requestSoloBattleSet.count(sock))
        {
            printf("Client : %d cancel solo battle match!\n", sock);
            m_requestSoloBattleSet.erase(sock);
        }
    }

    void MainServer::Response(int sock, CloseRequest login)
    {
        DeleteConnect(sock);
    }

    void MainServer::Response(int sock, LoginRequest login)
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
            m_messageProcesser.Send(sock, Protocol::LOGIN, std::move(response));
        }
        else {
            UserLogout(sock);
        }
    }

    void MainServer::Response(int sock, RegisterRequest rgst)
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
        m_messageProcesser.Send(sock, Protocol::REGISTER, std::move(response));
    }

    void MainServer::Response(int sock, MatchRequest match)
    {
        printf("Receive client : %d a new Match message!\n", sock);
        printf("MatchRequest:  ismatch = %d [true|false]   matchtype = %d\n\n", match.ismatch(), match.matchtype());
        if (match.ismatch())
        {
            if (2 == match.matchtype())
            {
                // 请求solo比赛
                RequestSoloBattle(sock);
            }
            else {
                printf("Request other battle!\n");
            }
        }
        else
        {
            if (2 == match.matchtype())
            {
                // 取消solo比赛
                CancelSoloBattle(sock);
            }
            else {
                printf("Cancel other battle!\n");
            }
        }
    }


}  // namespace WZRY





























