#include "ServerMessage.h"
#include "Server.h"
#include "ServerMessageProto.pb.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define PROTOCOL_LOGIN 1
#define PROTOCOL_REGISTER 2
#define PROTOCOL_MATCH 3

namespace WZRY
{

    ServerMessage::ServerMessage(Server& server) : m_server(server)
    {}

    void ServerMessage::Receive(int sock)
    {
        // receive message
        int len, ret;
        if (-1 == (ret = ReadN(sock, &len, 4)))
        {
            perror("ServerMessage::Receive");
            exit(EXIT_FAILURE);
        }
        len = ntohl(len);
        // client 断开连接
        if (0 == ret)
        {
            m_server.DeleteClient(sock);
            return;
        }
        if (ret < 4 || len < 4)
        {
            printf("ServerMessage::Receive error!\n");
            return;
        }
        int protocol;
        if (-1 == (ret = ReadN(sock, &protocol, 4)))
        {
            perror("ServerMessage::Receive");
            exit(EXIT_FAILURE);
        }
        else if (ret < 4)
        {
            printf("ServerMessage::Receive error!\n");
            return;
        }
        protocol = ntohl(protocol);
        char *buf = new char[len - 4];
        memset(buf, 0, len - 4);
        if (-1 == (ret = ReadN(sock, buf, len -4)))
        {
            perror("ServerMessage::Receive");
            exit(EXIT_FAILURE);
        }
        else if (ret < len - 4) {
            printf("ServerMessage::Receive error!\n");
            return;
        } else {
            /*
                根据协议序列化成对应的数据包，然后调用m_server.ResponseMessage(sock, Message);
                protocol :  1  login
                            2  register
                            3  match
            */
            printf("Receive client : %d a new message!\n", sock);
            switch (protocol)
            {
                case 1:
                    {
                        LoginMSG login;
                        login.ParseFromArray(buf, len - 4);
                        m_server.ResponseMessage(sock, login);
                        break;
                    }
                case 2:
                    {
                        RegisterMSG regist;
                        regist.ParseFromArray(buf, len - 4);
                        m_server.ResponseMessage(sock, regist);
                        break;
                    }
                case 3:
                    {
                        MatchMSG match;
                        match.ParseFromArray(buf, len - 4);
                        m_server.ResponseMessage(sock, match);
                        break;
                    }
            }
        }
        delete[] buf;
    }

    void ServerMessage::Send(int sock, LoginRSPS response)
    {
        size_t size = response.ByteSizeLong();
        void *buffer = malloc(size);
        response.SerializeToArray(buffer, size);
        Send(sock, PROTOCOL_LOGIN, buffer, size);
        free(buffer);
    }

    void ServerMessage::Send(int sock, RegisterRSPS response)
    {
        size_t size = response.ByteSizeLong();
        void *buffer = malloc(size);
        response.SerializeToArray(buffer, size);
        Send(sock, PROTOCOL_REGISTER, buffer, size);
    }

    void ServerMessage::Send(int sock, MatchRSPS response)
    {
        size_t size = response.ByteSizeLong();
        void *buffer = malloc(size);
        response.SerializeToArray(buffer, size);
        Send(sock, PROTOCOL_MATCH, buffer, size);
    }

    void ServerMessage::Send(int sock, int prtc, void* msg, int size)
    {
        printf("Send a message to client : %d   protocol : %d   size : %d\n", sock, prtc, size);
        int len = htonl(4 + size);
        WriteN(sock, &len, 4);
        prtc = htonl(prtc);
        WriteN(sock, &prtc, 4);
        WriteN(sock, msg, size);
        printf("Send finished\n\n");
    }

    int ServerMessage::ReadN(int sock, void* buf, size_t count)
    {
        size_t nleft = count;
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
                return count - nleft;
            bufp += nread;
            nleft -= nread;
        }
        return count;
    }

    int ServerMessage::WriteN(int sock, const void* buf, size_t count)
    {
        size_t nleft = count;
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
        return count;
    }

}  // namespace WZRY




