#include "MessageProcesser.h"
#include "Server.h"
#include "EnumClass.h"
#include "WZRYMessageProto.pb.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include <signal.h>

namespace WZRY
{

    MessageProcesser::MessageProcesser(Server* server) : m_server(server)
    {
        m_readBuff = new char[BUFF_SIZE];
        m_writeBuff = new char[BUFF_SIZE];
    }

    MessageProcesser::~MessageProcesser()
    {
        delete[] m_writeBuff;
        delete[] m_readBuff;
    }

    int MessageProcesser::ReadN(int sock, void* buf, size_t cnt)
    {
        size_t nleft = cnt;
        int nread;
        char *bufp = (char*)buf;
        while (nleft > 0)
        {
            if ((nread = read(sock, bufp, nleft)) < 0)
            {
                if (EINTR == errno)
                    continue;
                return -1;
            }
            else if (0 == nread)
                return cnt - nleft;
            bufp += nread;
            nleft -= nread;
        }
        return cnt;
    }

    int MessageProcesser::WriteN(int sock, const void* buf, size_t cnt)
    {
        size_t nleft = cnt;
        int nwrite;
        char *bufp = (char*)buf;
        while (nleft > 0)
        {
            if ((nwrite = write(sock, bufp, nleft)) < 0)
            {
                if (EINTR == errno)
                    continue;
                return -1;
            }
            else if (0 == nwrite)
                continue;
            bufp += nwrite;
            nleft -= nwrite;
        }
        return cnt;
    }

    bool MessageProcesser::RecvPeek(int sock)
    {
        int len, protocol, ret;
        ret = recv(sock, m_readBuff, BUFF_SIZE, MSG_PEEK);
        if (0 == ret)
        {
            /// client close  close是否是立即返回0
            protocol = 0;
        }
        else if ((-1 == ret && EINTR == errno) || ret <4)
        {
            return false;
        }
        else
        {
            memcpy(&len, m_readBuff, 4);
            len = ntohl(len);
            if (len + 4 > BUFF_SIZE)
            {
                printf("消息长度超出缓存区的长度！消息长度为：%d\n", len + 4);
                exit(EXIT_FAILURE);
            }
            if (len + 4 > ret)
            {
                /// 未接收完一条完整的信息
                return false;
            }
            /// 把一条完整的信息从socket中读走
            ReadN(sock, m_readBuff, len + 4);
            /// receive protocol
            memcpy(&protocol, m_readBuff + 4, 4);
            protocol = ntohl(protocol);
            /// 打印长度和协议
            printf("message length : %d    protocol : %d\n", len, protocol);

            if (protocol < 1 || protocol > 5)
            {
                printf("Receive a wrong protocol!\n");
                return true;
            }
        }
        printf("Receive client : %d a new message!\n", sock);
        switch (static_cast<Protocol>(protocol))
        {
            case Protocol::CLOSE:
                {
                    CloseRequest cls;
                    m_server->Response(sock, std::move(cls));
                    break;
                }
            case Protocol::LOGIN:
                {
                    LoginRequest login;
                    login.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(login));
                    break;
                }
            case Protocol::REGISTER:
                {
                    RegisterRequest regist;
                    regist.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(regist));
                    break;
                }
            case Protocol::MATCH:
                {
                    MatchRequest match;
                    match.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(match));
                    break;
                }
            case Protocol::MOVE:
                {
                    MoveData move;
                    move.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(move));
                    break;
                }
            case Protocol::SKILL:
                {
                    SkillData skill;
                    skill.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(skill));
                    break;
                }
            case Protocol::CHAT:
                {
                    ChatData chat;
                    chat.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(chat));
                    break;
                }
            case Protocol::FRAMEOVER:
                {
                    FrameOver frameover;
                    frameover.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(frameover));
                    break;
                }
            case Protocol::GAMEOVER:
                {
                    GameOver gameover;
                    gameover.ParseFromArray(m_readBuff + 8, len - 4);
                    m_server->Response(sock, std::move(gameover));
                    break;
                }
        }
        return true;
    }

    void MessageProcesser::Send(int sock, Protocol protocol, google::protobuf::Message *response)
    {
        size_t sz = response->ByteSizeLong();
        void *buff = malloc(sz);
        response->SerializeToArray(buff, sz);
        std::cout << "the message size = " << sz << "\n";
        Send(sock, static_cast<int>(protocol), buff, sz);
        free(buff);
    }

    /*
    void handle_sigpipe(int sig)
    {
        printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
        exit(EXIT_FAILURE);
    }
    */
    
    void MessageProcesser::Send(int sock, int protocol, void* msg, int sz)
    {
        if (8 + sz > BUFF_SIZE)
        {
            printf("The write message is too long. size = %d\n", 8 + sz);
            return;
        }
        int len = htonl(4 + sz);
        protocol = htonl(protocol);
        
        // 只调用一次WriteN，否则如果在客户端close之后还write的话会被信号量SIGPIPE中断服务器
        memcpy(m_writeBuff, &len, 4);
        memcpy(m_writeBuff + 4, &protocol, 4);
        memcpy(m_writeBuff + 8, msg, sz);
        if (-1 == WriteN(sock, m_writeBuff, 8 + sz))
        {
            CloseRequest cls;
            m_server->Response(sock, std::move(cls));
        }
        
        /*
        signal(SIGPIPE, handle_sigpipe);
        
        if ((-1 == WriteN(sock, &len, 4)) || (-1 == WriteN(sock, &protocol, 4)) || (-1 == WriteN(sock, msg, sz)))
        {
            CloseRequest cls;
            m_server->Response(sock, std::move(cls));
        }
        */
    }

}  // namespace WZRY






















