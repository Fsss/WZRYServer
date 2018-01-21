#include "Epoll.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

namespace WZRY
{

    Epoll::Epoll(int sz) : m_size(sz)
    {
        m_epollfd = epoll_create1(EPOLL_CLOEXEC);
        m_events = new epoll_event[sz];
    }

    Epoll::~Epoll()
    {
        close(m_epollfd);
        delete[] m_events;
    }

    int Epoll::EpollWait()
    {
        return epoll_wait(m_epollfd, m_events, m_size, -1);
    }

    int Epoll::EpollCtlAdd(int sock)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        // set socket O_NONBLOCK
        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0)|O_NONBLOCK);
        struct epoll_event event;
        event.data.fd = sock;
	    event.events = EPOLLIN|EPOLLET;
        return epoll_ctl(m_epollfd, EPOLL_CTL_ADD, sock, &event);
    }

    int Epoll::EpollCtlDel(int sock)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        struct epoll_event event;
	    event.data.fd = sock;
	    event.events = EPOLLIN|EPOLLET;
        return epoll_ctl(m_epollfd, EPOLL_CTL_DEL, sock, &event);
    }

    epoll_event& Epoll::GetEvent(int idx)
    {
        return m_events[idx];
    }


}  // namespace WZRY
























