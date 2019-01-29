/*
* Tencent is pleased to support the open source community by making Libco available.
* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, 
* software distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License.
*/

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "coctx.h"
#include "coroutine.h"

int coctx_make(tswCoCtx *ctx, tswCo_mkctx_func pfn, uintptr_t s, uintptr_t s1)
{
    char *sp;

    sp = (char *)((uintptr_t)ctx->stack.ss_sp + ctx->stack.ss_size); // 用户自定义的栈顶
    sp = (char *)(((uintptr_t)sp & -16L) - 8);

    ctx->regs[RSP] = sp;
    ctx->regs[RIP] = (char *)pfn;
    ctx->regs[RDI] = (char *)s;
    ctx->regs[RSI] = (char *)s1;

    return 0;
}
