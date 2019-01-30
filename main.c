#include <stdio.h>
#include "log.h"
#include "coroutine.h"

struct args {
    int n;
};

void func(tswCo_schedule *S, void *ud)
{
    struct args *arg;

    arg = (struct args *)ud;
    tswDebug("arg: %d", arg->n);
    if (tswCo_yield(S) < 0) {
        tswWarn("tswCo_yield error");
    }
    tswDebug("arg: %d", arg->n);
}

/*
 * main coroutine
*/
int main(int argc, char const *argv[])
{
    int co1;
    int co2;
    struct args arg1;
    struct args arg2;
    tswCo_schedule *S;

    S = tswCo_open();
    if (S == NULL) {
        tswWarn("tswCo_open error");
        return -1;
    }

    arg1.n = 1;
    arg2.n = 2;

    co1 = tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)&arg1);
    co2 = tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)&arg2);

    tswDebug("main coroutine");

    if (tswCo_resume(S, co1) < 0) {
        tswWarn("tswCo_resume error");
        return -1;
    }

    tswDebug("main coroutine");

    if (tswCo_resume(S, co2) < 0) {
        tswWarn("tswCo_resume error");
        return -1;
    }

    while (tswCo_status(S, co1) && tswCo_status(S, co2)) {
        tswDebug("main coroutine");
        tswCo_resume(S, co1);
        tswDebug("main coroutine");
        tswCo_resume(S, co2);
    }
    
    tswDebug("main coroutine");

    tswCo_close(S);
    return 0;
}
