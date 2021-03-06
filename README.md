## Reference

1、[Tencent/libco](https://github.com/Tencent/libco)

2、[cloudwu/coroutine](https://github.com/cloudwu/coroutine)

3、[rocaltair/mco](https://github.com/rocaltair/mco)

## Description

You can write synchronous code to achieve asynchronous effects! Enjoy it!

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

    tswCo_destroy(S);
    
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

    tswCo_run(S);
    
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

    tswCo_run(S);
    
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

    tswCo_destroy(S);

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
#include "socket.h"
#include "net.h"
#include "fd.h"

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
        if (n == 0) {
            if (tswCo_close(S, connfd) < 0) {
                tswWarn("tswCo_shutdown error");
            }
            break;
        } else {
            if (tswCo_send(S, connfd, buf, n, 0) < 0) {
                tswWarn("tswCo_send error");
            }
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
    tswCo_run(S);

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
    tswCo_destroy(S);

    return 0;
}
```

### example6

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
#include "socket.h"
#include "net.h"
#include "fd.h"

#define HOST "0.0.0.0"
#define PORT 9501
#define MAX_BUF_SIZE 1024
#define LISTENQ 16

char *response_str = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/html\r\nContent-Length: 11\r\n\r\nhello world\r\n";

void client_handle(tswCo_schedule *S, void *ud) {
    tswDebug("coroutine [%d] is running", S->running);
    int n;
    int connfd;
    char buf[MAX_BUF_SIZE];

    connfd = (int)(uintptr_t)ud;

    while (1) {
        if ((n = tswCo_recv(S, connfd, buf, MAX_BUF_SIZE, 0)) < 0) {
            tswWarn("tswCo_recv error: %s", strerror(errno));
        }
        if (n == 0) {
            if (tswCo_close(S, connfd) < 0) {
                tswWarn("tswCo_close error: %s", strerror(errno));
            }
            break;
        } else {
            if (tswCo_send(S, connfd, response_str, strlen(response_str), 0) < 0) {
                tswWarn("tswCo_send error: %s", strerror(errno));
            }
            if (tswCo_close(S, connfd) < 0) {
                tswWarn("tswCo_close error: %s", strerror(errno));
            }
            break;
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

    tswDebug("connection [%d] is running", S->running);

    while ((connfd = tswCo_accept(S, sockfd, (struct sockaddr *)&cliaddr, &len)) > 0) {
        tswDebug("a new connection [%d]", connfd);
        tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, client_handle, (void *)(uintptr_t)connfd);
    }
}

int start_service(tswCo_schedule *S)
{
    int sockfd;
    int on = 1;

    sockfd = tswSocket_create(TSW_SOCK_TCP);
    if (sockfd < 0) {
        tswWarn("tswSocket_create error");
        return -1;
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (tswSocket_bind(sockfd, TSW_SOCK_TCP, HOST, PORT) < 0) {
        tswWarn("tswSocket_bind error");
        return -1;
    }
    if (listen(sockfd, LISTENQ) < 0) {
        tswWarn("%s", strerror(errno));
    }

    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, listen_service, (void *)(uintptr_t)sockfd);
    tswCo_run(S);

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
    tswCo_destroy(S);

    return 0;
}
```

## Pressure test

### tsw_co

```shell
sh-4.2# ab -c 100 -n 100000 127.0.0.1:9501/
This is ApacheBench, Version 2.3 <$Revision: 1430300 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        
Server Hostname:        127.0.0.1
Server Port:            9501

Document Path:          /
Document Length:        13 bytes

Concurrency Level:      100
Time taken for tests:   7.538 seconds
Complete requests:      100000
Failed requests:        0
Write errors:           0
Total transferred:      9600000 bytes
HTML transferred:       1300000 bytes
Requests per second:    13266.45 [#/sec] (mean)
Time per request:       7.538 [ms] (mean)
Time per request:       0.075 [ms] (mean, across all concurrent requests)
Transfer rate:          1243.73 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    5  81.9      2    3122
Processing:    -4    2   2.2      2     209
Waiting:        0    1   2.0      1     208
Total:          0    8  82.0      3    3126

Percentage of the requests served within a certain time (ms)
  50%      3
  66%      5
  75%      6
  80%      7
  90%      7
  95%      8
  98%      9
  99%     11
 100%   3126 (longest request)
```



