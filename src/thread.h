#ifndef __ZCSERVER_THREAD_H__
#define __ZCSERVER_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>

namespace zcserver
{
    class Thread
    {
    public:
        typedef std::shared_ptr<Thread> ptr;
        // create the Thread
        // similar to std::thread model
        Thread(std::function<void()> cb, const std::string &name);
        ~Thread();

        // get some information
        const std::string &getName() const { return m_name; }
        pid_t getId() const { return m_id; }

        void join();

        // provide a reference to manipulate present running thread
        // return the pointer to present running thread
        static Thread* GetThis();
        // provide the thread name for logs
        // return the present running thread name
        static const std::string &GetName();

        // set the name of present running thread
        // mainly set for the main thread which is not created by users
        static void SetName(const std::string &name);

    private:
        // delete copy constructor
        Thread(const Thread&) = delete;
        Thread(const Thread&&) = delete;
        Thread &operator=(const Thread&) = delete;

        static void* run(void *arg);

        pid_t m_id = -1;                // process id, globally unique
        // m_thread = 0 means that the thread is not running
        pthread_t m_thread = 0;         // thread id, thread unique
        std::function<void()> m_cb;     // call back function
        std::string m_name;             // thread name
    };
}

#endif