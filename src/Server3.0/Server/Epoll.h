#ifndef _EPOLL_H_
#define _EPOLL_H_

#include <mutex>
#include <sys/epoll.h>


namespace WZRY
{

    class Epoll
    {
    public:
        Epoll(int);
        ~Epoll();
        int EpollWait();
        int EpollCtlAdd(int);
        int EpollCtlDel(int);
        epoll_event& GetEvent(int);
    private:
        Epoll(const Epoll&) = delete;
        Epoll& operator=(const Epoll&) = delete;
    private:
        int m_size;
        int m_epollfd;
        std::mutex m_mutex;
        epoll_event* m_events;
    };

}



#endif // _EPOLL_H_
















