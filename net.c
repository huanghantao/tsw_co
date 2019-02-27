#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <errno.h>
#include <string.h>
#include "net.h"
#include "log.h"
#include "epoll.h"

static int tswCo_setnonblock(int fd);

static int tswCo_setnonblock(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }
    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (flags < 0) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }

    return TSW_OK;
}

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

ssize_t tswCo_recv(tswCo_schedule *S, int connfd, void *buf, size_t len, int flags)
{
    int connfd_flags;
    int n = 0;
    int total = 0;
    int remain = len;

    connfd_flags = fcntl(connfd, F_GETFL, 0);
    if (connfd_flags < 0) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }
    if (!(connfd_flags & O_NONBLOCK)) { // if is not NONBLOCK
        if (tswCo_setnonblock(connfd) < 0) {
            tswWarn("tswCo_setnonblock error");
            return TSW_ERR;
        }
    }

    do {
        remain -= n;
        n = recv(connfd, buf + total, remain, flags);
        if (n < 0 && errno == EAGAIN) {
            if (tswCo_wait(S, connfd, TSW_FD_WRITE) < 0) {
                tswWarn("tswCo_wait error");
                return TSW_ERR;
            }
            n = 0;
        }
        total += n;
    } while (total < len);

    return len;
}

ssize_t tswCo_send(tswCo_schedule *S, int connfd, void *buf, size_t len, int flags)
{
    int connfd_flags;
    int n = 0;
    int total = 0;
    int remain = len;

    connfd_flags = fcntl(connfd, F_GETFL, 0);
    if (connfd_flags < 0) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }
    if (!(connfd_flags & O_NONBLOCK)) { // if is not NONBLOCK
        if (tswCo_setnonblock(connfd) < 0) {
            tswWarn("tswCo_setnonblock error");
            return TSW_ERR;
        }
    }

    do {
        remain -= n;
        n = send(connfd, buf + total, remain, flags);
        if (n < 0 && errno == EAGAIN) {
            if (tswCo_wait(S, connfd, TSW_FD_WRITE) < 0) {
                tswWarn("tswCo_wait error");
                return TSW_ERR;
            }
            n = 0;
        }
        total += n;
    } while (total < len);

    return len;
}