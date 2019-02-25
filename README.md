## Environment

```shell
cpu: x86_64
os: Linux
```

## Compile

```shell
make
```

## Execute

```shell
./main
```

## Example

### example1

```c
#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "coroutine.h"
#include "htimer.h"

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
    int co0;
    int co1;
    int co2;
    int co3;
    tswCo_schedule *S;

    S = tswCo_open();
    if (S == NULL) {
        tswWarn("tswCo_open error");
        return -1;
    }

    co0 = tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)0);
    co1 = tswCo_new(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)1);

    co2 = tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)2);
    co3 = tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func, (void *)(uintptr_t)3);

    do {
        if (tswCo_status(S, co0)) {
            tswCo_resume(S, co0);
        }
        if (tswCo_status(S, co1)) {
            tswCo_resume(S, co1);
        }
        if (tswCo_status(S, co2)) {
            tswCo_resume(S, co2);
        }
        if (tswCo_status(S, co3)) {
            tswCo_resume(S, co3);
        }
    } while (!(tswCo_status(S, co0) == TSW_CO_DEAD && tswCo_status(S, co1) == TSW_CO_DEAD));

    tswCo_close(S);
    
    return 0;
}

```

### example2

```c
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

```

