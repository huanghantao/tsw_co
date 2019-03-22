#ifndef TSW_COROUTINE_H_
#define TSW_COROUTINE_H_


#include "coctx.h"
#include "htimer.h"

enum {
    TSW_CO_DEAD = 0,
    TSW_CO_READY = 1,
    TSW_CO_RUNING = 2,
    TSW_CO_SUSPEND = 3,
};

#define TSW_CO_DEFAULT_ST_SZ 2*1024*1024
#define TSW_CO_DEFAULT_NUM 16
#define TSW_CO_CALL_DEPTH 8

typedef struct tswCo_schedule tswCo_schedule;
typedef struct tswCo tswCo;
typedef struct tswCoCtx_env tswCoCtx_env;
typedef struct timer_handler timer_handler;
typedef void (*tswCo_func)(tswCo_schedule *S, void *ud);
typedef void (*tswCo_mkctx_func)();

struct poll {
    int epollfd;
    int ncap;
    int nevents;
    void *events;
};

struct timer_handler {
    htimer_t timer;
    tswCo_schedule *S;
    int id;
};

struct tswCoCtx_env {
    tswCoCtx ctx;
    int co_id;
};

struct tswCo_schedule {
    tswCoCtx_env *env;
    int co_call_depth;
    htimer_mgr_t timer_mgr;
    int dst_sz;
    int running;
    int nco;
    int cap;
    tswCo **co;
    struct poll m_poll;
};

struct tswCo {
    int status;
    int st_sz;
    char *stack;
    tswCo_func func;
    tswCoCtx ctx;
    void *ud;
};

tswCo_schedule* tswCo_open();
void tswCo_sleep(tswCo_schedule *S, int ms);
int tswCo_run(tswCo_schedule *S);
int tswCo_scheduler(tswCo_schedule *S);
void tswCo_destroy(tswCo_schedule *S);
int tswCo_new(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud);
int tswCo_create(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud);
int tswCo_running(tswCo_schedule *S);
int tswCo_resume(tswCo_schedule *S, int id);
int tswCo_yield(tswCo_schedule *S);
int tswCo_status(tswCo_schedule *S, int id);

struct htimer_mgr_s *tswCo_get_timer_mgr(tswCo_schedule *S);
struct poll *tswCo_get_poll(tswCo_schedule *S);

#endif /* TSW_COROUTINE_H_ */
