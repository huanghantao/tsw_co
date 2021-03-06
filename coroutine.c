#include <string.h>
#include <stdint.h>
#include "coroutine.h"
#include "coctx.h"
#include "log.h"
#include "epoll.h"

static inline void tswCo_delete(tswCo *C);
static tswCo *new_tswCo(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud);
static int tswCo_cap_check(tswCo_schedule *S);
static void tswCo_entry(uintptr_t low, uintptr_t high);

static tswCo *new_tswCo(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud)
{
    tswCo *C;

    C = (tswCo *)malloc(sizeof(tswCo));
    if (C == NULL) {
        tswWarn("malloc error");
        return NULL;
    }

    C->func = func;
    if (st_sz < S->dst_sz) {
        C->st_sz = S->dst_sz;
    } else {
        C->st_sz = st_sz;
    }

    C->stack = (char *)malloc(C->st_sz);
    if (C->stack == NULL) {
        tswWarn("malloc error");
        free(C);
        return NULL;
    }
    C->status = TSW_CO_READY;
    C->ud = ud;
    return C;
}

static int tswCo_cap_check(tswCo_schedule *S)
{
    if (S->nco >= S->cap) {
        int old_cap = S->cap;
        tswCo **co;
        co = realloc(S->co, sizeof(tswCo *) * old_cap * 2);
        if (co == NULL) {
            tswWarn("realloc error");
            return TSW_ERR;
        }
        S->co = co;
        S->cap = 2 * old_cap;
        memset(S->co + old_cap, 0, sizeof(tswCo *) * old_cap);
    }

    return TSW_OK;
}

static void tswCo_delete(tswCo *C)
{
	free(C->stack);
    C->stack = NULL;
	free(C);
}

static void tswCo_entry(uintptr_t low, uintptr_t high)
{
    uintptr_t p;
    int id;
    tswCo *C;
    tswCoCtx tmp;
    tswCo_schedule *S;
    int call_depth;

    p = (uintptr_t)low | ((uintptr_t) high << 32);
    S = (tswCo_schedule *)p;

    id = S->running;
    C = S->co[id];
    C->func(S, C->ud);
    C->status = TSW_CO_DEAD;
    call_depth = S->co_call_depth;
    S->running = S->env[call_depth].co_id;
    S->co_call_depth--;
    S->nco--;
    coctx_swap(&tmp, &(S->env[call_depth].ctx));
}

/*
 * @function: init the coroutine schedule
*/
tswCo_schedule* tswCo_open()
{
    tswCo_schedule *S;

    S = malloc(sizeof(*S));
    if (S == NULL) {
        tswWarn("malloc error");
        return NULL;
    }
    S->dst_sz = TSW_CO_DEFAULT_ST_SZ;
    S->cap = TSW_CO_DEFAULT_NUM;
    S->env = (tswCoCtx_env *)malloc(sizeof(tswCoCtx_env) * TSW_CO_CALL_DEPTH);
    S->co_call_depth = -1;
    S->nco = 0;
    S->running = -1;
    htimer_mgr_init(&S->timer_mgr);
    tswCo_init_poll(S);
    S->co = malloc(sizeof(tswCo *) * S->cap);
    if (S->co == NULL) {
        tswWarn("malloc error");
        free(S);
        return NULL;
    }
    memset(S->co, 0, sizeof(tswCo *) * S->cap);
    return S;
}

static void timer_cb(htimer_t *handler)
{
    struct timer_handler *pth = (struct timer_handler *)handler;
    tswCo_resume(pth->S, pth->id);
}

void tswCo_sleep(tswCo_schedule *S, int ms)
{
    int id = S->running;
    struct timer_handler th;
    th.S = S;
    th.id = id;
    htimer_init(&S->timer_mgr, &th.timer);
    htimer_start(&th.timer, timer_cb, ms, 0);
    tswCo_yield(S);
}

int tswCo_run(tswCo_schedule *S)
{
    do {
        tswCo_poll(S);
    } while(1);
    return 0;
}

/*
 * @function: close the coroutine and coroutine schedule
*/
void tswCo_destroy(tswCo_schedule *S)
{
    int id;
    tswCo *C;

    for (id = 0; id < S->cap; id++) {
        C = S->co[id];
        if(C == NULL) {
            continue;
        }
        tswCo_delete(C);
    }

    free(S->co);
    S->co = NULL;
    tswCo_release_poll(S);
    free(S);
    S = NULL;
}

/*
 * @st_sz: the stack size that this coroutine can use
 * @func: the task that this coroutine run
 * @ud: parameters required for task execution
 * 
 * @return coroutine id
 * 
 * @notice: if S->nco >= S->cap, we need to double the S->cap
*/
int tswCo_new(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud)
{
    int i;
    int id;
    tswCo *C;

    C = (tswCo *)malloc(sizeof(tswCo));
    if (C == NULL) {
        tswWarn("malloc error");
        return TSW_ERR;
    }

    if (tswCo_cap_check(S) < 0) {
        tswWarn("tswCo_cap_check error");
        return TSW_ERR;
    }

    C = new_tswCo(S, st_sz, func, ud);
    if (C == NULL) {
        tswWarn("new_tswCo error");
        return TSW_ERR;
    }

    for (i = 0; i <= S->cap; i++) {
        id = (i + S->nco) % S->cap;
        if (S->co[id] == NULL || S->co[id]->status == TSW_CO_DEAD) {
            break;
        }
    }

    S->nco++;
    S->co[id] = C;

    return id;
}

/*
 * tswCo_new and tswCo_resume
*/
int tswCo_create(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud)
{
    int co;

    co = tswCo_new(S, st_sz, func, ud);
    if (co < 0) {
        tswWarn("tswCo_new error");
        return TSW_ERR;
    }
    if (tswCo_resume(S, co) < 0) {
        tswWarn("tswCo_resume error");
        return TSW_ERR;
    }

    return co;
}

/*
 * @function: get the running coroutine
*/
int tswCo_running(tswCo_schedule *S)
{
    return S->running;
}

/*
 * @function: start or start a coroutine again
 * 
 * @id: coroutine id
 * 
 * @notice: 
 * 1、can't have a work coroutine running.
 * 2、id must be valid
 * 3、consider the first resume(READY) and the non-first resume(SUSPEND)
*/
int tswCo_resume(tswCo_schedule *S, int id)
{
    tswCo *C;
    int call_depth;

    if (id < 0 || id >= S->cap) {
        return TSW_ERR;
    }
    
    C = S->co[id];
    if (C == NULL) {
        tswWarn("C == NULL");
        return TSW_ERR;
    }
    if (C->status == TSW_CO_RUNING) {
        tswWarn("don't resume co while running");
        return TSW_ERR;
    }

    switch(C->status) {
    case TSW_CO_READY:
        coctx_get(&C->ctx);
        C->ctx.stack.ss_sp = C->stack;
        C->ctx.stack.ss_size = C->st_sz;
        C->status = TSW_CO_RUNING;
        if (S->co_call_depth + 1 >= TSW_CO_CALL_DEPTH) {
            tswWarn("coroutine call is too deep");
            return TSW_ERR;
        }
        call_depth = ++(S->co_call_depth);
        S->env[call_depth].co_id = S->running;
        S->running = id;
        coctx_make(&C->ctx, (tswCo_mkctx_func)tswCo_entry, (uint32_t)(uintptr_t)S, (uint32_t)((uintptr_t)S>>32));
        coctx_swap(&(S->env[call_depth].ctx), &C->ctx);
        break;
    case TSW_CO_SUSPEND:
        S->running = id;
        C->status = TSW_CO_RUNING;
        if (S->co_call_depth + 1 >= TSW_CO_CALL_DEPTH) {
            tswWarn("coroutine call is too deep");
            return TSW_ERR;
        }
        call_depth = ++(S->co_call_depth);
        S->env[call_depth].co_id = S->running;
        S->running = id;
        coctx_swap(&(S->env[call_depth].ctx), &C->ctx);
        break;
    }

    return TSW_OK;
}

int tswCo_yield(tswCo_schedule *S)
{
    int id;
    int call_depth;
    tswCo *C;
    
    id = S->running;
    if (id < 0 || id >= S->cap) {
        tswWarn("id < 0 || id >= S->cap");
        return TSW_ERR;
    }

    C = S->co[id];
    if (C == NULL) {
        tswWarn("C == NULL");
        return TSW_ERR;
    }
    if (C->status != TSW_CO_RUNING) {
        tswWarn("C->status != TSW_CO_RUNING");
        return TSW_ERR;
    }
    C->status = TSW_CO_SUSPEND;
    call_depth = S->co_call_depth;
    S->running = S->env[call_depth].co_id;
    S->co_call_depth--;
    coctx_swap(&C->ctx, &(S->env[call_depth].ctx));

    return TSW_OK;
}

int tswCo_status(tswCo_schedule *S, int id)
{
    tswCo *C;

    if (id < 0 || id >= S->cap) {
        tswWarn("id >= 0 && id < S->cap");
        return TSW_ERR;
    }
    C = S->co[id];
    if (C == NULL) {
        return TSW_CO_DEAD;
    }
        
    return C->status;
}

htimer_mgr_t *tswCo_get_timer_mgr(tswCo_schedule *S)
{
    return &S->timer_mgr;
}

struct poll *tswCo_get_poll(tswCo_schedule *S)
{
    return &S->m_poll;
}
