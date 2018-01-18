#include "SoloMessage.h"
#include "SoloBattle.h"
#include "ServerMessageProto.pb.h"

#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

namespace WZRY
{

    SoloMessage::SoloMessage(SoloBattle& soloBattle) : m_soloBattle(soloBattle)
    {}

    void SoloMessage::Receive(int sock)
    {
        // receive message
        int len, ret;
        if (-1 == (ret = ReadN(sock, &len, 4)))
        {
            perror("MessageCenter::Receive");
            exit(EXIT_FAILURE);
        }
        len = ntohl(len);
        if (ret < 4 || len < 4)
        {
            printf("MessageCenter::Receive error!\n");
            return;
        }
        int protocol;
        if (-1 == (ret = ReadN(sock, &protocol, 4)))
        {
            perror("MessageCenter::Receive");
            exit(EXIT_FAILURE);
        }
        else if (ret < 4)
        {
            printf("MessageCenter::Receive error!\n");
            return;
        }
        protocol = ntohl(protocol);
        char *buf = new char[len - 4];
        memset(buf, 0, len - 4);
        if (-1 == (ret = ReadN(sock, buf, len -4)))
        {
            perror("MessageCenter::Receive");
            exit(EXIT_FAILURE);
        }
        else if (ret < len - 4) {
            printf("MessageCenter::Receive error!\n");
            return;
        } else {
            // 根据协议序列化成对应的数据包，然后调用m_server.ResponseMessage(sock, Message);
        }
        delete[] buf;
    }

    void SoloMessage::Send(int sock, ChatRSPS response)
    {

    }

    void SoloMessage::Send(int sock, MoveRSPS response)
    {

    }

    void SoloMessage::Send(int sock, SkillRSPS response)
    {
        
    }

    int SoloMessage::ReadN(int sock, void* buf, size_t count)
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

    int SoloMessage::WriteN(int sock, const void* buf, size_t count)
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



