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
