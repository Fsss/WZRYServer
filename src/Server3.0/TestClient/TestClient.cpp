#include "WZRYMessageProto.pb.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024

using namespace std;
using namespace WZRY;


template <typename T>
void TSend(int sock, int prtc, T message)
{
    size_t size = message.ByteSizeLong();
    void *buff = malloc(size);
    message.SerializeToArray(buff, size);
    // send len
    int len = htonl(4 + size);
    send(sock, &len, 4, 0);
    // send protocol
    prtc = htonl(prtc);
    send(sock, &prtc, 4, 0);
    // send message
    cout << "the message size = " << size << "\n";
    send(sock, buff, size, 0);
    // free buff
    free(buff);
}



int cacheLen = 0;
char *cacheBuff = new char[BUFF_SIZE << 1];
char *recvBuff = new char[BUFF_SIZE];

void ReceiveMessage(int sock)
{
    int ret = recv(sock, recvBuff, BUFF_SIZE, 0);
    memcpy(cacheBuff + cacheLen, recvBuff, ret);
    cacheLen += ret;
    printf("ret = %d     cacheLen = %d\n", ret, cacheLen);
    while (cacheLen >= 4)
    {
        int len;
        memcpy(&len, cacheBuff, 4);
        len = ntohl(len);
        printf("len = %d\n", len);
        if (cacheLen < len + 4)
        {
            return;
        }
        int protocol;
        memcpy(&protocol, cacheBuff + 4, 4);
        protocol = ntohl(protocol);
        char *buff = new char[len - 4];
        memcpy(buff, cacheBuff + 8, len - 4);
        for (int i = len + 4; i < cacheLen; ++i)
            cacheBuff[i - len - 4] = cacheBuff[i];
        cacheLen -= len + 4;
        printf("len = %d    protocol = %d\n", len, protocol);
        switch (protocol)
        {
            case 3:
                {
                    MatchResponse match;
                    match.ParseFromArray(buff, len - 4);
                    cout << match.issuccess() << "   "  << match.id() << "   " << match.matchmessage() << "\n";
                    break;
                }
            case 7:
                {
                    printf("Receive a frameover message!\n");
                    break;
                }
        }
        delete[] buff;
    }
}


int main()
{
    const char* ip = "127.0.0.1";
    short port = 12345;

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));


    MatchRequest match;
    match.set_ismatch(true);
    match.set_matchtype(2);
    TSend(client_fd, 3, match);

    int cnt = 0;
    while (true)
    {
        ReceiveMessage(client_fd);
        printf("Receive!                               %d\n", ++cnt);
        if (cnt == 5000)
        {
            GameOver gameover;
            TSend(client_fd, 8, gameover);
            break;
        }
    }

    
    sleep(10);
    
    close(client_fd);
    return 0;
}


/*

g++ -g -pthread -lprotobuf -Wall -std=c++11 TestClient.cpp WZRYMessageProto.pb.cc -o TestClient

*/


