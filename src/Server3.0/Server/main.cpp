#include "Server.h"

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

    WZRY::Server server(port);

    server.ServerLoop();

    return 0;
}

/*

protoc -I=./ --cpp_out=./ WZRYMessageProto.proto

g++ -g -D_GLIBCXX_USE_NANOSLEEP -pthread -lprotobuf -Wall -std=c++11 Epoll.cpp main.cpp MessageProcesser.cpp Server.cpp WZRYMessageProto.pb.cc -o RunServer

*/
