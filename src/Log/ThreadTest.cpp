#include <pthread.h>
#include <iostream>
#include <unistd.h>
using namespace std;

class A
{
public:
    static void* ALoop(void* args)
    {
        A *a = (A*)args;
        a->m_isRunning = true;
        while (a->m_isRunning)
        {
            std::cout << "aaaa\n";
            sleep(1);
        }
        return 0;
    }
    A() {}
    void Start()
    {
        pthread_create(&m_pid, NULL, A::ALoop, (void*)&m_isRunning);
    }
    void Stop()
    {
        m_isRunning = false;
    }
private:
    bool m_isRunning;
    pthread_t m_pid;
};

int main()
{
    A a;
    a.Start();

    sleep(6);
    a.Stop();

    return 0;
}

