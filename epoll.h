#include <sys/epoll.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "htimer.h"
#include "coroutine.h"

struct htimer_mgr_s;
struct htimer_mgr_s *tswCo_get_timer_mgr(tswCo_schedule *S);
struct poll *tswCo_get_poll(tswCo_schedule *S);

static void tswCo_init_mpoll(tswCo_schedule *S)
{
    size_t sz;
    struct poll *m_poll;

    m_poll = tswCo_get_poll(S);
    /* epoll_create(n), n is unused scince Linux 2.6.8*/
    m_poll->epollfd = epoll_create(32000);
    m_poll->ncap = 16;
    m_poll->nevents = 0;

    sz = sizeof(struct epoll_event) * m_poll->ncap;
    m_poll->events = malloc(sz);
    memset(m_poll->events, 0, sz);
}

static void tswCo_release_mpoll(tswCo_schedule *S)
{
    struct poll *m_poll;

    m_poll = tswCo_get_poll(S);
    free(m_poll->events);
    m_poll->events = NULL;
    m_poll->nevents = 0;
    m_poll->ncap = 0;
}

static int tswCo_poll(tswCo_schedule *S)
{
    int n;
    struct poll *m_poll;
    struct epoll_event *events;
    int epollfd;
    struct htimer_mgr_s *timer_mgr;
    int next;

    m_poll = tswCo_get_poll(S);
    events = m_poll->events;
    epollfd = m_poll->epollfd;
    timer_mgr = tswCo_get_timer_mgr(S);
    next = htimer_next_timeout(timer_mgr);

    if (next < 0)
        next = 1000;

    if (tswCo_running(S) >= 0) {
        tswWarn("tswCo_running(S) >= 0");
        return TSW_ERR;
    }

    n = epoll_wait(epollfd, events, m_poll->ncap, next);
    htimer_perform(timer_mgr);
    if (n <= 0) {
        return 0;
    }

    return 0;
}
