#include <string.h>
#include <stdint.h>
#include "coroutine.h"
#include "coctx.h"
#include "log.h"

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

    p = (uintptr_t)low | ((uintptr_t) high << 32);
    S = (tswCo_schedule *)p;

    id = S->running;
    C = S->co[id];
    C->func(S, C->ud);
    C->status = TSW_CO_DEAD;
    S->running = -1;
    S->nco--;
    coctx_swap(&tmp, &S->main);
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
    S->nco = 0;
    S->running = -1;
    S->co = malloc(sizeof(tswCo *) * S->cap);
    if (S->co == NULL) {
        tswWarn("malloc error");
        free(S);
        return NULL;
    }
    memset(S->co, 0, sizeof(tswCo *) * S->cap);
    return S;
}

/*
 * @function: close the coroutine and coroutine schedule
*/
void tswCo_close(tswCo_schedule *S)
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
        if (S->co[id] == NULL) {
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

    if (id < 0 || id >= S->cap) {
        return TSW_ERR;
    }
    if (S->running != -1) {
        tswWarn("S->running != -1");
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
        S->running = id;
        coctx_make(&C->ctx, (tswCo_mkctx_func)tswCo_entry, (uint32_t)(uintptr_t)S, (uint32_t)((uintptr_t)S>>32));
        coctx_swap(&S->main, &C->ctx);
        break;
    case TSW_CO_SUSPEND:
        S->running = id;
        C->status = TSW_CO_RUNING;
        coctx_swap(&S->main, &C->ctx);
        break;
    }

    return TSW_OK;
}

int tswCo_yield(tswCo_schedule *S)
{
    int id;
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
    S->running = -1;
    coctx_swap(&C->ctx, &S->main);

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