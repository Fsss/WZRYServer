#include <stdio.h>
#include <stdlib.h>

#include "Server.h"



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


