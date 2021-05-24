#ifndef __ZCSERVER_UTIL_H__
#define __ZCSERVER_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>

namespace zcserver
{
    pid_t GetThreadId();
    uint32_t GetFiberId();
}

#endif