#ifndef _SOLOBATTLE_H_
#define _SOLOBATTLE_H_

#include "Player.h"
#include "Epoll.h"
#include "SoloMessage.h"
#include "ServerMessageProto.pb.h"

#include <string>
#include <pthread.h>

namespace WZRY
{

    class SoloBattle
    {
    public:
        SoloBattle(int, int, std::string, std::string);
        ~SoloBattle();
        void Start();
        void Stop();
        void ResponseMessage(int, ChatRSPS);
        void ResponseMessage(int, MoveRSPS);
        void ResponseMessage(int, SkillRSPS);
    private:
        SoloBattle(const SoloBattle&) = delete;
        SoloBattle& operator=(const SoloBattle&) = delete;
        static void* SoloThreadLoop(void* arg);
    private:
        bool m_isRunning;
        pthread_t m_pId;
        Epoll m_epoll;
        Player m_player[2];
        SoloMessage m_soloMessage;
    };

}  // namespace WZRY

#endif  // _SOLOBATTLE_H_


