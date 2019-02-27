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

### example3

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "log.h"
#include "coroutine.h"
#include "htimer.h"
#include "fd.h"

#define MAX_BUF_SIZE 1024
#define MAX_FILENAME_LEN 20

struct args {
    char filename[MAX_FILENAME_LEN + 1];
    char buf[MAX_BUF_SIZE];
    int n;
};

void func(tswCo_schedule *S, void *ud)
{
    struct args *arg = (struct args *)ud;
    int fd;

    fd = open(arg->filename, O_RDWR);
    tswCo_write(S, fd, arg->buf, arg->n);
}

/*
 * main coroutine
*/
int main(int argc, char const *argv[])
{
    tswCo_schedule *S;
    struct args arg1 = {"test1.txt", "aaaaaaaaaa", 10};
    struct args arg2 = {"test2.txt", "bbbbbbbbbb", 10};

    S = tswCo_open();
    if (S == NULL) {
        tswWarn("tswCo_open error");
        return -1;
    }

    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func, &arg1);
    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func, &arg2);

    tswCo_run(S, 0);
    
    return 0;
}

```

### example4

```c
#include "log.h"
#include "coroutine.h"

void func3(tswCo_schedule *S, void *ud)
{
    tswDebug("coroutine [%d] is running", tswCo_running(S));
}

void func2(tswCo_schedule *S, void *ud)
{
    tswDebug("coroutine [%d] is running", tswCo_running(S));
    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func3, ud);
    tswDebug("coroutine [%d] is running", tswCo_running(S));
}

void func1(tswCo_schedule *S, void *ud)
{
    tswDebug("coroutine [%d] is running", tswCo_running(S));
    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func2, ud);
    tswDebug("coroutine [%d] is running", tswCo_running(S));
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

    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, func1, (void *)(uintptr_t)1);

    tswCo_close(S);

    return 0;
}
```

### example5

```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "coroutine.h"
#include "htimer.h"
#include "socket.h"
#include "net.h"

#define HOST "127.0.0.1"
#define PORT 9501
#define MAX_BUF_SIZE 1024
#define LISTENQ 10

void client_handle(tswCo_schedule *S, void *ud) {
    int n;
    int connfd;
    char buf[MAX_BUF_SIZE];

    connfd = (int)(uintptr_t)ud;
    while (1) {
        if ((n = tswCo_recv(S, connfd, buf, MAX_BUF_SIZE, 0)) < 0) {
            tswWarn("tswCo_recv error");
        }
        if (tswCo_send(S, connfd, buf, n, 0) < 0) {
            tswWarn("tswCo_send error");
        }
    }
}

void listen_service(tswCo_schedule *S, void *ud)
{
    int sockfd;
    int connfd;
    struct sockaddr_in cliaddr;
    socklen_t len;

    sockfd = (int)(uintptr_t)ud;

    while ((connfd = tswCo_accept(S, sockfd, (struct sockaddr *)&cliaddr, &len)) > 0) {
        tswDebug("a new connection [%d]", connfd);
        tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, client_handle, (void *)(uintptr_t)connfd);
    }
}

int start_service(tswCo_schedule *S)
{
    int sockfd;

    sockfd = tswSocket_create(TSW_SOCK_TCP);
    if (sockfd < 0) {
        tswWarn("tswSocket_create error");
        return -1;
    }
    if (tswSocket_bind(sockfd, TSW_SOCK_TCP, HOST, PORT) < 0) {
		tswWarn("tswSocket_bind error");
        return -1;
	}
    if (listen(sockfd, LISTENQ) < 0) {
        tswWarn("%s", strerror(errno));
    }

    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, listen_service, (void *)(uintptr_t)sockfd);
    tswCo_run(S, 0);

    return 0;
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
    start_service(S);
    tswCo_close(S);

    return 0;
}
```

