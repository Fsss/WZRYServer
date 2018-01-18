#include "Server.h"
#include "Epoll.h"
#include "ServerMessageProto.pb.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LISTEN_SIZE 100
#define MAX_EVENTS 100
#define BUFF_SIZE 1024
#define MAX_CONNECT 99

namespace WZRY
{

    Server::Server(int port) : m_port(port), m_epoll(LISTEN_SIZE), m_serverMessage(*this)
    {
        // socket
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

        // set REUSEADDR
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
            printf("Server epoll ctl error!\n");
            return;
        }
        // 初始user
        m_users["123"] = "123";
        printf("Server listen in port : %d ...\n", port);
    }

    Server::~Server()
    {
        close(m_listenfd);
    }

    void Server::ServerLoop()
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
                    DeleteClient(eventFd);
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
                    m_connetcSet.insert(newClient);
                } else {
                    m_serverMessage.Receive(eventFd);
                }
            }
        }
    }

    void Server::DeleteClient(int sock)
    {
        printf("Client : %d close the connect.\n", sock);
        m_epoll.EpollCtlDel(sock);
        close(sock);
        DeleteSolo(sock);
        m_connetcSet.erase(sock);
    }
    /*
        type： 1   登录成功
               2   密码错误
               3   账号不存在
    */
    void Server::ResponseMessage(int sock, LoginMSG login)
    {
        printf("Receive client : %d a new Login message!\n", sock);
        printf("LoginMSG:  username = %s   passwd = %s\n\n", login.username().c_str(), login.passwd().c_str());
        LoginRSPS response;
        if (m_users.end() != m_users.find(login.username()))
        {
            if (login.passwd() == m_users[login.username()])
            {
                response.set_type(1);
                response.set_message("登录成功");
            }
            else
            {
                response.set_type(2);
                response.set_message("密码错误");
            }
        }
        else
        {
            response.set_type(3);
            response.set_message("账号不存在");
        }
        m_serverMessage.Send(sock, response);
    }

    /*
        type： 1  注册成功
               2  用户已存在
    */
    void Server::ResponseMessage(int sock, RegisterMSG rgst)
    {
        printf("Receive client : %d a new Register message!\n", sock);
        printf("RegisterMSG:  newname = %s   newpasswd = %s\n\n", rgst.newname().c_str(), rgst.newpasswd().c_str());
        RegisterRSPS response;
        if (m_users.end() != m_users.find(rgst.newname()))
        {
            response.set_type(2);
            response.set_message("账号已存在");
        }
        else
        {
            m_users[rgst.newname()] = rgst.newpasswd();
            response.set_type(1);
            response.set_message("注册成功");
        }
        m_serverMessage.Send(sock, response);
    }

    /*
        type： 1  正在匹配  true
               2  取消匹配  false
    */
    void Server::ResponseMessage(int sock, MatchMSG match)
    {
        printf("Receive client : %d a new Match message!\n", sock);
        printf("MatchMSG:  ismatch = %d [true|false]   type = %d\n\n", match.ismatch(), match.type());
        MatchRSPS response;
        response.set_success(match.ismatch());
        response.set_type(match.type());
        m_serverMessage.Send(sock, response);
        // 先响应再插入set中
        if (match.ismatch())
        {
            if (2 == match.type())
            {
                AddSolo(sock);
            }
        }
        else
        {
            if (2 == match.type())
            {
                DeleteSolo(sock);
            }
        }
    }

    void Server::AddSolo(int sock)
    {
        printf("Client : %d require solo battle match!\n", sock);
        m_soloSet.insert(sock);
        if (m_soloSet.size() >= 2)
        {
            printf("Begin a solo battle!\n");
            /*
            int player1 = *(m_soloSet.begin());
            int player2 = *(++m_soloSet.begin());
            SoloBattle solo(player1, player2, m_loginUsers[player1], m_loginUsers[player2]);
            m_soloSet.erase(player1);
            m_soloSet.erase(player2);
            m_epoll.EpollCtlDel(player1);
            m_epoll.EpollCtlDel(player2);
            solo.Start();
            //m_soloBattle.insert(solo);
            //m_soloBattle.insert(std::move(solo));
            */
        }
    }

    void Server::DeleteSolo(int sock)
    {
        if (m_soloSet.count(sock))
        {
            printf("Client : %d cancel solo battle match!\n", sock);
            m_soloSet.erase(sock);
        }
    }


}  // namespace WZRY



