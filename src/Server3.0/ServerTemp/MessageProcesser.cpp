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



namespace WZRY
{

    MessageProcesser::MessageProcesser(Server* server) : m_server(server)
    {
        m_buff = new char[BUFF_SIZE];
    }

    MessageProcesser::~MessageProcesser()
    {
        delete[] m_buff;
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
        ret = recv(sock, m_buff, BUFF_SIZE, MSG_PEEK);
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
            memcpy(&len, m_buff, 4);
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
            ReadN(sock, m_buff, len + 4);
            /// receive protocol
            memcpy(&protocol, m_buff + 4, 4);
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
                    login.ParseFromArray(m_buff + 8, len - 4);
                    m_server->Response(sock, std::move(login));
                    break;
                }
            case Protocol::REGISTER:
                {
                    RegisterRequest regist;
                    regist.ParseFromArray(m_buff + 8, len - 4);
                    m_server->Response(sock, std::move(regist));
                    break;
                }
            case Protocol::MATCH:
                {
                    MatchRequest match;
                    match.ParseFromArray(m_buff + 8, len - 4);
                    m_server->Response(sock, std::move(match));
                    break;
                }
            case Protocol::CHAT:
                {
                    ChatRequest chat;
                    chat.ParseFromArray(m_buff + 8, len - 4);
                    m_server->Response(sock, std::move(chat));
                    break;
                }
            case Protocol::SKILL:
                {
                    SkillRequest skill;
                        skill.ParseFromArray(m_buff + 8, len - 4);
                        m_server->Response(sock, std::move(skill));
                    break;
                }
        }
        return true;
    }

    void MessageProcesser::Send(int sock, int protocol, void* msg, int sz)
    {
        int len = htonl(4 + sz);
        WriteN(sock, &len, 4);
        protocol = htonl(protocol);
        WriteN(sock, &protocol, 4);
        WriteN(sock, msg, sz);
    }

}  // namespace WZRY






















