#ifndef TSW_COROUTINE_H_
#define TSW_COROUTINE_H_


#include "coctx.h"

enum {
    TSW_CO_DEAD = 0,
    TSW_CO_READY = 1,
    TSW_CO_RUNING = 2,
    TSW_CO_SUSPEND = 3,
};

#define TSW_CO_DEFAULT_ST_SZ 2*1024*1024
#define TSW_CO_DEFAULT_NUM 16

typedef struct tswCo_schedule tswCo_schedule;
typedef struct tswCo tswCo;
typedef void (*tswCo_func)(tswCo_schedule *S, void *ud);
typedef void (*tswCo_mkctx_func)();

struct tswCo_schedule {
    tswCoCtx main;
    int dst_sz;
    int running;
    int nco;
    int cap;
    tswCo **co;
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
void tswCo_close(tswCo_schedule *S);
int tswCo_new(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud);
int tswCo_running(tswCo_schedule *S);
int tswCo_resume(tswCo_schedule *S, int id);

#endif /* TSW_COROUTINE_H_ */
