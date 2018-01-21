#ifndef _BATTLESERVER_H_
#define _BATTLESERVER_H_

#include "Epoll.h"
#include "Server.h"
#include "MessageProcesser.h"
#include "WZRYMessageProto.pb.h"

namespace WZRY
{

    class BattleServer : public Server
    {
    public:
        BattleServer(Epoll*, int, ...);
        ~BattleServer();
        virtual void Response(int, MoveData) override;
        virtual void Response(int, SkillData) override;
        virtual void Response(int, ChatData) override;
        virtual void Response(int, GameOver) override;

        void BeginGame();
    private:
        BattleServer(const BattleServer&) = delete;
        BattleServer& operator=(const BattleServer&) = delete;
        int GetId(int);
    private:
        int m_size;
        int m_gameOver;
        bool m_isRunning;
        int *m_playerfd;
        Epoll m_epoll;
        Epoll *m_MSEpoll;
        MessageProcesser m_messageProcesser;
    };

}  // namespace WZRY



#endif  // _BATTLESERVER_H_


