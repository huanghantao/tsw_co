#ifndef TSW_EPOLL_H_
#define TSW_EPOLL_H_

#include <sys/epoll.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "htimer.h"
#include "coroutine.h"

enum {
    TSW_FD_WRITE = 0,
    TSW_FD_READ = 1,
};

struct htimer_mgr_s;
struct poll *tswCo_get_poll(tswCo_schedule *S);
void tswCo_init_mpoll(tswCo_schedule *S);
void tswCo_release_mpoll(tswCo_schedule *S);
int tswCo_poll(tswCo_schedule *S);
int tswCo_wait(tswCo_schedule *S, int fd, int flag);

#endif /* TSW_EPOLL_H_ */
