#include "util.h"

namespace zcserver
{
    pid_t GetThreadId()
    {
        // return pthread_self();
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId()
    {
        return 0;
    }
}
