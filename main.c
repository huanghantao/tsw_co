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

    while ((connfd = tswCo_accept(S, sockfd, (struct sockaddr *)&cliaddr, &len)) > 0) {
        tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, client_handle, (void *)(uintptr_t)connfd);
    }
}

void start_service(tswCo_schedule *S, void *ud)
{
    int sockfd;
    int on = 1;

    sockfd = tswSocket_create(TSW_SOCK_TCP);
    if (sockfd < 0) {
        tswWarn("tswSocket_create error");
    }
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (tswSocket_bind(sockfd, TSW_SOCK_TCP, HOST, PORT) < 0) {
        tswWarn("tswSocket_bind error");
    }
    if (listen(sockfd, LISTENQ) < 0) {
        tswWarn("%s", strerror(errno));
    }

    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, listen_service, (void *)(uintptr_t)sockfd);
}
/**
 * main coroutine
 * you can define the task coroutine here
 */

void tswCo_main(tswCo_schedule *S, void *ud)
{
    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, start_service, (void *)NULL);
}

int main(int argc, char const *argv[])
{
    tswCo_schedule *S;

    S = tswCo_open();
    if (S == NULL) {
        tswWarn("tswCo_open error");
        return -1;
    }

    tswCo_create(S, TSW_CO_DEFAULT_ST_SZ, tswCo_main, (void *)NULL);
    tswCo_run(S);
    tswCo_destroy(S);

    return 0;
}
