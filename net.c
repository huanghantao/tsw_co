#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <string.h>
#include "net.h"
#include "log.h"
#include "epoll.h"

int tswCo_accept(tswCo_schedule *S, int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int connfd;
    struct sockaddr_in sa;
    socklen_t len;

    len = sizeof(sa);
	if((connfd = accept(sockfd, (void*)&sa, &len)) < 0) {
        if (connfd == EAGAIN) {
            if (tswCo_wait(S, sockfd, TSW_FD_READ) < 0) {
                tswWarn("tswCo_wait error");
                return TSW_ERR;
            }
        } else {
            tswWarn("%s", strerror(errno));
            return TSW_ERR;
        }
	}

    return connfd;
}