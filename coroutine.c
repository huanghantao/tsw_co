#include <string.h>
#include "coroutine.h"
#include "log.h"

static inline void delete_tswCo(tswCo *C);
static  tswCo *new_tswCo(tswCo_schedule *S, int st_sz, tswCo_func func, void *ud);
static int tswCo_cap_check(tswCo_schedule *S);

static inline void delete_tswCo(tswCo *C)
{
    free(C->stack);
    C->stack = NULL;
    free(C);
}

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

void tswCo_close(tswCo_schedule *S)
{
    int id;
    tswCo *C;

    for (id = 0; id < S->cap; id++) {
        C = S->co[id];
        if(C == NULL) {
            continue;
        }
        delete_tswCo(C);
        S->co[id] = NULL;
    }

    free(S->co);
    S->co = NULL;
    free(S);
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

/*
 *
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

    return TSW_OK;
}