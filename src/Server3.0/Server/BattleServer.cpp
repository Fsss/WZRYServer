#include "BattleServer.h"
#include "Epoll.h"
#include "WZRYMessageProto.pb.h"

#include <thread>
#include <chrono>
#include <stdarg.h>


namespace WZRY
{

    BattleServer::BattleServer(Epoll *MSEpoll, int size, ...)
        : m_size(size), m_gameOver(0), m_isRunning(false), m_epoll(size), m_MSEpoll(MSEpoll), m_messageProcesser(this)
    {
        m_playerfd = new int[size];
        va_list args;
        va_start(args, size);
        while (size--)
        {
            m_playerfd[size] = va_arg(args, int);
            m_MSEpoll->EpollCtlDel(m_playerfd[size]);
            m_epoll.EpollCtlAdd(m_playerfd[size]);
        }
        va_end(args);
    }

    BattleServer::~BattleServer()
    {
        delete[] m_playerfd;
    }


    void BattleServer::Response(int sock, MoveData move)
    {
        for (int i = 0; i < m_size; ++i)
            m_messageProcesser.Send(m_playerfd[i], Protocol::MOVE, &move);
    }
    void BattleServer::Response(int sock, SkillData skill)
    {
        for (int i = 0; i < m_size; ++i)
            m_messageProcesser.Send(m_playerfd[i], Protocol::SKILL, &skill);
    }
    void BattleServer::Response(int sock, ChatData chat)
    {
        for (int i = 0; i < m_size; ++i)
            m_messageProcesser.Send(m_playerfd[i], Protocol::CHAT, &chat);
    }
    void BattleServer::Response(int sock, GameOver gameover)
    {
        // 游戏结束转移client socket
        ++m_gameOver;
        m_epoll.EpollCtlDel(sock);
        m_MSEpoll->EpollCtlAdd(sock);
    }


    void BattleServer::BeginGame()
    {
        if (m_isRunning) return;
        std::thread game([&]()
            {
                // 通知匹配成功
                MatchResponse matchSuccess;
                matchSuccess.set_issuccess(true);
                matchSuccess.set_matchmessage("匹配成功");
                for (int i = 0; i < m_size; ++i)
                {
                    matchSuccess.set_id(GetId(m_playerfd[i]));
                    m_messageProcesser.Send(m_playerfd[i], Protocol::MATCH, &matchSuccess);
                }
                m_isRunning = true;
                while (m_isRunning)
                {
                    clock_t lastTime, nowTime;
                    FrameOver frameOver;
                    lastTime = nowTime = clock();
                    while (true)  // 直到游戏结束事件触发break
                    {
                        int n = m_epoll.EpollWait();
                        for (int i = 0; i < n; ++i)
                        {
                            epoll_event event = m_epoll.GetEvent(i);
                            int eventFd = event.data.fd;
                            int eventEvents = event.events;
                            if ((eventEvents & EPOLLERR) || (eventEvents & EPOLLHUP) || (!(eventEvents & EPOLLIN)))
                            {
                                printf("clientfd : %d error\n", eventFd);
                                /*
                                    处理客户端掉线
                                */
                                continue;
                            } else {
                                while (m_messageProcesser.RecvPeek(eventFd));
                            }
                        }
                        if (m_gameOver == m_size)
                        {
                            break;
                        }
                        //发送结束包
                        nowTime = clock();
                        if (nowTime - lastTime >= 20)
                        {
                            lastTime = nowTime;
                            for (int i = 0; i < m_size; ++i)
                                m_messageProcesser.Send(m_playerfd[i], Protocol::FRAMEOVER, &frameOver);
                        }
                    }
                }
                m_isRunning = false;
            });
        game.detach();
    }

    int BattleServer::GetId(int sock)
    {
        for (int i = 0; i < m_size; ++i)
        if (sock == m_playerfd[i])
            return (i + 1);
        return 0;
    }


}  // namespace WZRY




