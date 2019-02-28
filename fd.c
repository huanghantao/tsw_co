#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "fd.h"
#include "log.h"
#include "coroutine.h"
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

ssize_t tswCo_read(tswCo_schedule *S, int fd, void *buf, size_t count)
{
    int flags;
    int n = 0;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }
    if (!(flags & O_NONBLOCK)) { // if is not NONBLOCK
        if (tswCo_setnonblock(fd) < 0) {
            tswWarn("tswCo_setnonblock error");
            return TSW_ERR;
        }
    }

    while ((n = read(fd, buf, count)) < 0 && errno == EAGAIN) {
        if (tswCo_wait(S, fd, TSW_FD_READ) < 0) {
            tswWarn("tswCo_wait error");
            return TSW_ERR;
        }
    }

    return n;
}

ssize_t tswCo_write(tswCo_schedule *S, int fd, const void *buf, size_t count)
{
    int flags;
    int n = 0;
    int total = 0;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }
    if (!(flags & O_NONBLOCK)) { // if is not NONBLOCK
        if (tswCo_setnonblock(fd) < 0) {
            tswWarn("tswCo_setnonblock error");
            return TSW_ERR;
        }
    }

    for (total = 0; total < count; total += n) {
        while ((n = write(fd, (char *)buf + total, count - total)) < 0 && errno == EAGAIN) {
            if (tswCo_wait(S, fd, TSW_FD_READ) < 0) {
                tswWarn("tswCo_wait error");
                return TSW_ERR;
            }
        }
        if (n < 0) {
            return n;
        }
        if (n == 0) {
            break;
        }
    }

    return total;
}

int tswCo_close(tswCo_schedule *S, int fd)
{
    if (close(fd) < 0) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }
    return TSW_OK;
}