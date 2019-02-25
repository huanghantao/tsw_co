#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "coroutine.h"
#include "htimer.h"

void func(tswCo_schedule *S, void *ud)
{
    int n;

    n = (int)(uintptr_t)ud;

    while (1) {
        tswDebug("coroutine [%d] is running", tswCo_running(S));
        tswCo_sleep(S, n * 1000);
    }
}

/*
 * main coroutine
*/
int main(int argc, char const *argv[])
{
    tswCo_schedule *S;

    S = tswCo_open();
    if (S == NULL) {
        tswWarn("tswCo_open error");
        return -1;
    }

    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)1);
    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)1);

    tswCo_run(S, 0);
    
    return 0;
}
