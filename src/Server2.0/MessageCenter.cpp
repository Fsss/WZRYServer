#include "MessageCenter.h"
#include "Server.h"
#include "EnumClass.h"

namespace WZRY
{

    MessageCenter::MessageCenter(Server *server) : m_server(server)
    {
    }

    void MessageCenter::Receive(int sock)
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



    template <typename M>
    void ServerMessage::Send(int sock, MessageProtocol protocol, M message)
    {
        size_t size = message.ByteSizeLong();
        void *buffer = malloc(size);
        message.SerializeToArray(buffer, size);
        Send(sock, static_cast<int>(protocol), buffer, size);
        free(buffer);
    }

    void ServerMessage::Send(int sock, int protocol, void* msg, int size)
    {
        int len = htonl(4 + size);
        WriteN(sock, &len, 4);
        protocol = htonl(protocol);
        WriteN(sock, &protocol, 4);
        WriteN(sock, msg, size);
    }

    int MessageCenter::ReadN(int sock, void* buf, size_t count)
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

    int MessageCenter::WriteN(int sock, const void* buf, size_t count)
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



