#include <stdio.h>
#include "log.h"
#include "coroutine.h"

void func(tswCo_schedule *S, void *ud)
{
    int n;
    int i;

    n = (int)(uintptr_t)ud;
    for (i = 0; i < n; i++) {
        tswDebug("coroutine [%d] is running", tswCo_running(S));
        tswCo_yield(S);
    }
}

/*
 * main coroutine
*/
int main(int argc, char const *argv[])
{
    int co1;
    int co2;
    tswCo_schedule *S;

    S = tswCo_open();
    if (S == NULL) {
        tswWarn("tswCo_open error");
        return -1;
    }

    co1 = tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)3);
    co2 = tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)5);

    if (tswCo_resume(S, co1) < 0) {
        tswWarn("tswCo_resume error");
        return -1;
    }
    if (tswCo_resume(S, co2) < 0) {
        tswWarn("tswCo_resume error");
        return -1;
    }

    while (1) {
        if (tswCo_status(S, co1)) {
            tswCo_resume(S, co1);
        }
        if (tswCo_status(S, co2)) {
            tswCo_resume(S, co2);
        }
        if (tswCo_status(S, co1) == TSW_CO_DEAD && tswCo_status(S, co2) == TSW_CO_DEAD) {
            break;
        }
    }

    tswCo_close(S);
    return 0;
}
