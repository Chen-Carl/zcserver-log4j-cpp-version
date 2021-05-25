#include "../src/log.h"
#include "../src/thread.h"
#include <iostream>

void fun1();
void fun2();

int count = 0;
zcserver::RWMutex s_mutex;

std::shared_ptr<zcserver::Logger> g_logger = ZCSERVER_LOG_ROOT();

int main()
{
    ZCSERVER_LOG_INFO(g_logger) << "thread test begin";
    std::vector<zcserver::Thread::ptr> thrs;
    for (int i = 0; i < 5; i++)
    {
        zcserver::Thread::ptr thr(new zcserver::Thread(&fun1, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for (int i = 0; i < 5; i++)
    {
        thrs[i]->join();
    }

    ZCSERVER_LOG_INFO(g_logger) << "thread test end";
    ZCSERVER_LOG_INFO(g_logger) << "count = " << count;
    return 0;
}

void fun1()
{
    ZCSERVER_LOG_INFO(g_logger) << "name: " << zcserver::Thread::GetName()
                                << " this.name: " << zcserver::Thread::GetThis()->getName()
                                << " id: " << zcserver::GetThreadId()
                                << " this.id: " << zcserver::Thread::GetThis()->getId();
    for (int i = 0; i < 10; i++)
    {
        zcserver::RWMutex::WriteLock lock(s_mutex);
        count++;
    }
}

void fun2()
{

}