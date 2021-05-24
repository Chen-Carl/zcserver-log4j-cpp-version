#include "thread.h"
#include "log.h"

namespace zcserver
{
    // reference to current thread
    static thread_local Thread* t_thread = nullptr;
    // although each thread has the attribute "m_name", visit a static variable is more efficient
    static thread_local std::string t_thread_name = "UNKNOWN";

    static std::shared_ptr<Logger> g_logger = ZCSERVER_LOG_NAME("system");

    Thread::Thread(std::function<void()> cb, const std::string &name): m_cb(cb), m_name(name)
    {
        if (name.empty())
        {
            m_name = "UNKNOWN";
        }
        // the second paramter is set to be nullptr to use the default attribute
        // begin at function run
        int rt = pthread_create(&m_thread, nullptr, &run, this);
        if (rt)
        {
            ZCSERVER_LOG_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt << " name=" << name;
            throw std::logic_error("pthread_create error");
        }
    }

    Thread::~Thread()
    {
        if (m_thread)
        {
            // enable sub thread to release resources automatically
            // probable bug about getting static variable, which is not attached to this thread
            pthread_detach(m_thread);
        }
    }

    // busy join
    void Thread::join()
    {
        if (m_thread)
        {
            int rt = pthread_join(m_thread, nullptr);
            if (rt)
            {
                ZCSERVER_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    // 3 static methods
    Thread *Thread::GetThis()
    {
        return t_thread;
    }
    
    const std::string &Thread::GetName()
    {
        return t_thread_name;
    }

    void Thread::SetName(const std::string &name)
    {
        if (t_thread)
        {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    void* Thread::run(void *arg)
    {
        Thread* thread = (Thread*)arg;
        t_thread = thread;
        t_thread_name = thread->m_name;

        thread->m_id = GetThreadId();
        // name the thread, 16 characters maximum
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());
        // get the running function
        // using swap to destroy the function in the thread
        // reduce the reference of shared_ptr
        std::function<void()> cb;
        cb.swap(thread->m_cb);
        cb();
        return 0;
    }
}