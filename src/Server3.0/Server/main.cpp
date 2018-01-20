#include "MainServer.h"

#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Input port!\n");
        return 0;
    }

    short port = atoi(argv[1]);

    WZRY::MainServer mainServer(port);

    mainServer.MainServerLoop();

    return 0;
}

/*

g++ -g -pthread -lprotobuf -Wall -std=c++11 Epoll.cpp main.cpp MainServer.cpp MessageProcesser.cpp Server.cpp WZRYMessageProto.pb.cc -o RunServer

*/
