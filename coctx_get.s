.globl coctx_get
#if !defined( __APPLE__ )
.type  coctx_get, @function
#endif
coctx_get:
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

	movq (%rsp), %rcx
	movq %rcx, 104(%rdi)
	leaq 8(%rsp), %rcx
	movq %rcx, 96(%rdi)
	ret
	