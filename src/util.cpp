#include "util.h"

namespace zcserver
{
    pid_t GetThreadId()
    {
        return pthread_self();
    }

    uint32_t GetFiberId()
    {
        return 0;
    }
}
