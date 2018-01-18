#include "Epoll.h"
#include <sys/epoll.h>
#include <unistd.h>


namespace WZRY
{

    Epoll::Epoll(int size) : m_size(size)
    {
        m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        m_events = new epoll_event[size];
    }

    Epoll::~Epoll()
    {
        close(m_epoll_fd);
        delete[] m_events;
    }

    int Epoll::EpollWait()
    {
        return epoll_wait(m_epoll_fd, m_events, m_size, -1);
    }

    int Epoll::EpollCtlAdd(int sock)
    {
        struct epoll_event event;
	    event.data.fd = sock;
	    event.events = EPOLLIN|EPOLLET;
        return epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, sock, &event);
    }

    int Epoll::EpollCtlDel(int sock)
    {
        struct epoll_event event;
	    event.data.fd = sock;
	    event.events = EPOLLIN|EPOLLET;
        return epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, sock, &event);
    }

    epoll_event& Epoll::GetEvent(int idx)
    {
        return m_events[idx];
    }

}  // namespace WZRY

