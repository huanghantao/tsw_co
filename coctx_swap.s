.globl coctx_swap
#if !defined( __APPLE__ )
.type  coctx_swap, @function
#endif
coctx_swap:
	movq %rbx, 72(%rdi)
	movq %rbp, 64(%rdi)
	movq %r12, 16(%rdi)
	movq %r13, 24(%rdi)
	movq %r14, 32(%rdi)
	movq %r15, 40(%rdi)

	movq %rdi, 48(%rdi)
	movq %rsi, 56(%rdi)
	movq %rdx, 80(%rdi)
	movq %rcx, 88(%rdi)
	movq %r8, (%rdi)
	movq %r9, 8(%rdi)

    // store rip
	movq (%rsp), %rcx
	movq %rcx, 104(%rdi)
    // store rsp
	leaq 8(%rsp), %rcx
	movq %rcx, 96(%rdi)

    // load rsp
    movq	96(%rsi), %rsp
	movq	72(%rsi), %rbx
	movq	64(%rsi), %rbp
	movq	16(%rsi), %r12
	movq	24(%rsi), %r13
	movq	32(%rsi), %r14
	movq	40(%rsi), %r15
    
    // load rip
    movq	104(%rsi), %rcx
	pushq	%rcx

    movq	48(%rsi), %rdi
	movq	80(%rsi), %rdx
	movq	88(%rsi), %rcx
	movq	(%rsi), %r8
	movq	8(%rsi), %r9

    movq	56(%rsi), %rsi

	ret
	