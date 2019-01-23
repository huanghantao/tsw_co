#include <string.h>
#include "coroutine.h"

static void delete_tswCo(tswCo *C);

static void delete_tswCo(tswCo *C)
{
    free(C->stack);
    C->stack = NULL;
    free(C);
}

tswCo_schedule* tswCo_open()
{
    tswCo_schedule *S;

    S = malloc(sizeof(*S));
    if (S == NULL) {
        return NULL;
    }
    S->dst_sz = TSW_CO_DEFAULT_ST_SZ;
    S->cap = TSW_CO_DEFAULT_NUM;
    S->nco = 0;
    S->running = -1;
    S->co = malloc(sizeof(tswCo *) * S->cap);
    if (S->co == NULL) {
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