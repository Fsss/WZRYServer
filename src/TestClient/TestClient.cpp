#include "ServerMessageProto.pb.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
    send(sock, buff, size, 0);
    // free buff
    free(buff);
}

void Receive(int sock)
{
    int len, prtc;
    // recv len
    recv(sock, &len, 4, 0);
    len = ntohl(len);
    // recv prtc
    recv(sock, &prtc, 4, 0);
    prtc = ntohl(prtc);

    printf("receive     %d  %d\n", len, prtc);
    void *buff = malloc(len - 4);
    int recvLen = recv(sock, buff, len - 4, 0);
    printf("recv len = %d\n", recvLen);

    switch (prtc)
    {
        case 1:
            {
                LoginRSPS response;
                response.ParseFromArray(buff, len - 4);
                cout << response.type() << "   " << response.message() << "\n";
                break;
            }
        case 2:
            {
                RegisterRSPS response;
                response.ParseFromArray(buff, len - 4);
                cout << response.type() << "   " << response.message() << "\n";
                break;
            }
        case 3:
            {
                MatchRSPS response;
                response.ParseFromArray(buff, len - 4);
                cout << response.success() << "   " << response.type() << "\n";
                break;
            }
    }
    // free buff
    free(buff);
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

    
    // test login
    LoginMSG login;
    login.set_username("123");
    login.set_passwd("1234");
    TSend(client_fd, 1, login);
    Receive(client_fd);
    
    // test login
    login.set_username("Fsss");
    login.set_passwd("1234");
    TSend(client_fd, 1, login);
    Receive(client_fd);
    

    // test login
    login.set_username("123");
    login.set_passwd("123");
    TSend(client_fd, 1, login);
    Receive(client_fd);

    MatchMSG match;
    match.set_ismatch(true);
    match.set_type(2);
    TSend(client_fd, 3, match);
    Receive(client_fd);


    // test register
    
    RegisterMSG regs;
    regs.set_newname("Fsss");
    regs.set_newpasswd("12345");
    TSend(client_fd, 2, regs);
    Receive(client_fd);

    //test register Fsss
    login.set_username("Fsss");
    login.set_passwd("12345");
    TSend(client_fd, 1, login);
    Receive(client_fd);

    regs.set_newname("Fsss");
    regs.set_newpasswd("123");
    TSend(client_fd, 2, regs);
    Receive(client_fd);
    

    while (true)
    {
        cout << "Wait one second!\n";
        sleep(1);
    }

    close(client_fd);
    return 0;
}


/*

g++ -g -pthread -lprotobuf  -Wall -std=c++11 TestClient.cpp ServerMessageProto.pb.cc -o TestClient

*/


