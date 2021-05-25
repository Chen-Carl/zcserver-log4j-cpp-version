#ifndef __ZCSERVER_THREAD_H__
#define __ZCSERVER_THREAD_H__

#include <thread>
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>

namespace zcserver
{
    // lock_guard template
    template <class T>
    class ScopedLockImpl
    {
    public:
        ScopedLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.lock();
            m_locked = true;
        }

        ~ScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T &m_mutex;
        bool m_locked;
    };

    class Semaphore
    {
    public:
        // initialize Semaphore count
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        // count minus one
        // if count equals zero, block the thread
        void wait();
        // count add one
        // if a thread is waiting, wake up the thread
        void notify();

    private:
        Semaphore(const Semaphore &) = delete;
        Semaphore(const Semaphore &&) = delete;
        Semaphore operator=(const Semaphore &) = delete;

        sem_t m_semaphore;
    };

    template <class T>
    class ReadScopedLockImpl
    {
    public:
        ReadScopedLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.rdlock();
            m_locked = true;
        }

        ~ReadScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T &m_mutex;
        bool m_locked;
    };

    template <class T>
    class WriteScopedLockImpl
    {
    public:
        WriteScopedLockImpl(T &mutex) : m_mutex(mutex)
        {
            m_mutex.wrlock();
            m_locked = true;
        }

        ~WriteScopedLockImpl()
        {
            unlock();
        }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T &m_mutex;
        bool m_locked;
    };

    // read-write mutex
    class RWMutex
    {
    public:
        typedef ReadScopedLockImpl<RWMutex> ReadLock;
        typedef WriteScopedLockImpl<RWMutex> WriteLock;

    public:
        RWMutex()
        {
            pthread_rwlock_init(&m_lock, nullptr);
        }

        ~RWMutex()
        {
            pthread_rwlock_destroy(&m_lock);
        }

        void rdlock()
        {
            pthread_rwlock_rdlock(&m_lock);
        }

        void wrlock()
        {
            pthread_rwlock_wrlock(&m_lock);
        }

        void unlock()
        {
            pthread_rwlock_unlock(&m_lock);
        }

    private:
        pthread_rwlock_t m_lock;
    };

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

        Semaphore m_semaphore;
    };
}

#endif