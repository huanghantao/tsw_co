#include <errno.h>
#include <string.h>
#include <errno.h>
#include "epoll.h"
#include "htimer.h"
#include "coroutine.h"
#include "log.h"

static uint64_t touint64(int fd, int id);
static void fromuint64(uint64_t v, int *fd, int *id);

static uint64_t touint64(int fd, int id)
{
    uint64_t ret = 0;
    ret |= ((uint64_t)fd) << 32;
    ret |= ((uint64_t)id);

    return ret;
}

static void fromuint64(uint64_t v, int *fd, int *id)
{
    *fd = (int)(v >> 32);
    *id = (int)(v & 0xffffffff);
}

void tswCo_init_poll(tswCo_schedule *S)
{
    size_t sz;
    struct poll *tsw_poll;

    tsw_poll = tswCo_get_poll(S);
    /* epoll_create(n), n is unused scince Linux 2.6.8*/
    tsw_poll->epollfd = epoll_create(32000);
    tsw_poll->ncap = 16;
    tsw_poll->nevents = 0;

    sz = sizeof(struct epoll_event) * tsw_poll->ncap;
    tsw_poll->events = malloc(sz);
    memset(tsw_poll->events, 0, sz);
}

void tswCo_release_poll(tswCo_schedule *S)
{
    struct poll *tsw_poll;

    tsw_poll = tswCo_get_poll(S);
    free(tsw_poll->events);
    tsw_poll->events = NULL;
    tsw_poll->nevents = 0;
    tsw_poll->ncap = 0;
}

int tswCo_poll(tswCo_schedule *S)
{
    int n;
    int i;
    struct poll *tsw_poll;
    struct epoll_event *events;
    int epollfd;
    struct htimer_mgr_s *timer_mgr;
    int next;

    tsw_poll = tswCo_get_poll(S);
    events = tsw_poll->events;
    epollfd = tsw_poll->epollfd;
    timer_mgr = tswCo_get_timer_mgr(S);
    next = htimer_next_timeout(timer_mgr);

    if (next < 0) {
        next = 1000;
    }

    n = epoll_wait(epollfd, events, tsw_poll->ncap, next);
    htimer_perform(timer_mgr);
    if (n <= 0) {
        return 0;
    }

    for (i = 0; i < n; i++) {
        int fd;
        int id;
        struct epoll_event *p = &events[i];
        uint64_t u64 = p->data.u64;

        fromuint64(u64, &fd, &id);
        tsw_poll->nevents--;
        tswCo_resume(S, id);
    }

    return TSW_OK;
}

int tswCo_wait(tswCo_schedule *S, int fd, int flag)
{
    int id;
    struct poll *m_poll;
    struct epoll_event ev;
    struct epoll_event *new_events;

    id = tswCo_running(S);
    if (id < 0) {
        tswWarn("no coroutine is running");
        return TSW_ERR;
    }
    m_poll = tswCo_get_poll(S);

    if (flag != TSW_FD_READ && flag != TSW_FD_WRITE) {
        tswWarn("flag type is error");
        return TSW_ERR;
    }

    if (m_poll->nevents + 1 > m_poll->ncap) {
        new_events = realloc(m_poll->events, m_poll->ncap * 2);
        if (new_events != NULL) {
            m_poll->events = new_events;
            m_poll->ncap *= 2;
        }
    }

    ev.events = flag == TSW_FD_READ ? EPOLLIN : EPOLLOUT;
    ev.data.u64 = touint64(fd, id);

    if (epoll_ctl(m_poll->epollfd, EPOLL_CTL_ADD, fd, &ev) < 0 && (errno != EEXIST)) {
        tswWarn("%s", strerror(errno));
        return TSW_ERR;
    }
    m_poll->nevents++;
    tswCo_yield(S);

    return TSW_OK;
}