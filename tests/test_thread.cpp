#include "../src/log.h"
#include "../src/thread.h"

void fun1();
void fun2();

std::shared_ptr<zcserver::Logger> g_logger = ZCSERVER_LOG_ROOT();

int main(int argc, char **argv)
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
    return 0;
}

void fun1()
{
    ZCSERVER_LOG_INFO(g_logger) << "name: " << zcserver::Thread::GetName()
                                << " this.name: " << zcserver::Thread::GetThis()->getName()
                                << " id: " << zcserver::GetThreadId()
                                << " this.id: " << zcserver::Thread::GetThis()->getId();
}

void fun2()
{

}