#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "coroutine.h"
#include "socket.h"
#include "net.h"
#include "fd.h"

void tswCo1(tswCo_schedule *S, void *ud)
{
    int i;

    for (i = 0; i < 5; i++) {
        tswDebug("coroutine [%d]", S->running);
        tswCo_yield(S);
    }
}

void tswCo2(tswCo_schedule *S, void *ud)
{
    int i;

    for (i = 0; i < 5; i++) {
        tswDebug("coroutine [%d]", S->running);
        tswCo_yield(S);
    }
}

/**
 * main coroutine
 * you can define the task coroutine here
 */

void tswCo_main(tswCo_schedule *S, int argc, char **argv)
{
    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, tswCo1, (void *)NULL);
    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, tswCo2, (void *)NULL);
}
