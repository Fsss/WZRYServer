#include "SoloBattle.h"
#include "ServerMessageProto.pb.h"

#include <string>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

namespace WZRY
{

    SoloBattle::SoloBattle(int sock1, int sock2, std::string name1, std::string name2)
        : m_isRunning(false), m_epoll(2), m_soloMessage(*this)
    {
        m_player[0].Init(1, sock1, name1);
        m_player[1].Init(2, sock2, name2);
        m_epoll.EpollCtlAdd(sock1);
        m_epoll.EpollCtlAdd(sock2);
    }

    SoloBattle::~SoloBattle()
    {
        Stop();
    }

    void SoloBattle::Start()
    {
        if (pthread_create(&m_pId, NULL, SoloBattle::SoloThreadLoop, (void*)this))
            printf("Create pthread successed!\n");
        else
            printf("Create pthread failed!\n");
    }
    
    void SoloBattle::Stop()
    {
        m_isRunning = false;
    }

    void* SoloBattle::SoloThreadLoop(void* args)
    {
        SoloBattle *battle = (SoloBattle*)args;
        battle->m_isRunning = true;
        while (battle->m_isRunning)
        {
            // game loop
            printf("Solo Battle Loop");
            sleep(1);
        }
        return 0;
    }

}  // namespace WZRY


