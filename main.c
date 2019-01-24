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
}

int main(int argc, char const *argv[])
{
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

    tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)&arg1);
    tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)&arg2);

    tswCo_close(S);
    return 0;
}
