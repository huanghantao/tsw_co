#ifndef TSW_CONTEXT_H_
#define TSW_CONTEXT_H_


#include <stdlib.h>
#include <stdint.h>
#include "coctx.h"

enum {
    R8 = 0,
    R9,
    R12,
    R13,
    R14,
    R15,
    RDI,
    RSI,
    RBP,
    RBX,
    RDX,
    RCX,
    RSP,
    RIP
};

typedef struct tswCoStack tswCoStack;
typedef struct tswCoCtx tswCoCtx;
typedef void (*tswCo_mkctx_func)();

struct tswCoStack {
    char *ss_sp;
    int ss_size;
};

struct tswCoCtx {
    void *regs[14];
    tswCoStack stack;
};

extern void coctx_get(tswCoCtx *ctx) asm("coctx_get");
int coctx_make(tswCoCtx *ctx, tswCo_mkctx_func pfn, uintptr_t s, uintptr_t s1);
extern void coctx_swap(tswCoCtx *main, tswCoCtx *ctx) asm("coctx_swap");

#endif /* TSW_CONTEXT_H_ */