	.file	"quadatric2.c"
	.text
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC16:
	.string	"Tiempo requerido solucion Double: %f\n"
	.align 8
.LC17:
	.string	"Tiempo requerido solucion Float: %f\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB22:
	.cfi_startproc
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	movl	$40000000, %edi
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$56, %rsp
	.cfi_def_cfa_offset 112
	call	malloc
	movl	$40000000, %edi
	movq	%rax, %r15
	call	malloc
	movl	$40000000, %edi
	movq	%rax, %r14
	call	malloc
	movl	$80000000, %edi
	movq	%rax, %r13
	call	malloc
	movl	$80000000, %edi
	movq	%rax, %r12
	call	malloc
	movl	$80000000, %edi
	movq	%rax, %rbp
	call	malloc
	vbroadcastss	.LC1(%rip), %xmm0
	leaq	40000000(%r15), %rdx
	movq	%rax, %rbx
	movq	%r15, %rax
.L2:
	vmovups	%xmm0, (%rax)
	vmovups	%xmm0, 16(%rax)
	addq	$32, %rax
	cmpq	%rax, %rdx
	jne	.L2
	vbroadcastss	.LC3(%rip), %xmm0
	movq	%r14, %rax
	leaq	40000000(%r14), %rdx
.L3:
	vmovups	%xmm0, (%rax)
	vmovups	%xmm0, 16(%rax)
	addq	$32, %rax
	cmpq	%rax, %rdx
	jne	.L3
	vbroadcastss	.LC5(%rip), %xmm0
	movq	%r13, %rax
	leaq	40000000(%r13), %rdx
.L4:
	vmovups	%xmm0, (%rax)
	vmovups	%xmm0, 16(%rax)
	addq	$32, %rax
	cmpq	%rdx, %rax
	jne	.L4
	vmovddup	.LC7(%rip), %xmm0
	movq	%r12, %rax
	leaq	80000000(%r12), %rdx
.L5:
	vmovupd	%xmm0, (%rax)
	vmovupd	%xmm0, 16(%rax)
	addq	$32, %rax
	cmpq	%rdx, %rax
	jne	.L5
	vmovddup	.LC9(%rip), %xmm0
	movq	%rbp, %rax
	leaq	80000000(%rbp), %rdx
.L6:
	vmovupd	%xmm0, (%rax)
	vmovupd	%xmm0, 16(%rax)
	addq	$32, %rax
	cmpq	%rdx, %rax
	jne	.L6
	vmovddup	.LC11(%rip), %xmm0
	movq	%rbx, %rax
	leaq	80000000(%rbx), %rdx
.L7:
	vmovupd	%xmm0, (%rax)
	vmovupd	%xmm0, 16(%rax)
	addq	$32, %rax
	cmpq	%rax, %rdx
	jne	.L7
	xorl	%esi, %esi
	leaq	32(%rsp), %rdi
	call	gettimeofday
	vxorps	%xmm3, %xmm3, %xmm3
	vcvtsi2sdq	40(%rsp), %xmm3, %xmm0
	vdivsd	.LC12(%rip), %xmm0, %xmm0
	vmovsd	.LC13(%rip), %xmm4
	vcvtsi2sdq	32(%rsp), %xmm3, %xmm1
	movl	$300, %edx
	vxorps	%xmm5, %xmm5, %xmm5
	vaddsd	%xmm1, %xmm0, %xmm6
	vmovsd	%xmm6, (%rsp)
.L8:
	xorl	%eax, %eax
	.p2align 6
	.p2align 4
	.p2align 3
.L10:
	vcvtss2sd	(%r15,%rax), %xmm3, %xmm1
	vcvtss2sd	0(%r13,%rax), %xmm3, %xmm2
	vcvtss2sd	(%r14,%rax), %xmm3, %xmm0
	vmulsd	%xmm4, %xmm1, %xmm1
	vmulsd	%xmm2, %xmm1, %xmm1
	vfmsub132sd	%xmm0, %xmm1, %xmm0
	vcvtsd2ss	%xmm0, %xmm0, %xmm0
	vucomiss	%xmm0, %xmm5
	ja	.L32
.L9:
	addq	$4, %rax
	cmpq	$40000000, %rax
	jne	.L10
	decl	%edx
	jne	.L8
	xorl	%esi, %esi
	leaq	32(%rsp), %rdi
	call	gettimeofday
	vxorps	%xmm3, %xmm3, %xmm3
	vcvtsi2sdq	40(%rsp), %xmm3, %xmm0
	vdivsd	.LC12(%rip), %xmm0, %xmm0
	vcvtsi2sdq	32(%rsp), %xmm3, %xmm1
	xorl	%esi, %esi
	leaq	32(%rsp), %rdi
	vaddsd	%xmm1, %xmm0, %xmm0
	vsubsd	(%rsp), %xmm0, %xmm7
	vmovsd	%xmm7, (%rsp)
	call	gettimeofday
	vxorps	%xmm3, %xmm3, %xmm3
	vcvtsi2sdq	40(%rsp), %xmm3, %xmm0
	vdivsd	.LC12(%rip), %xmm0, %xmm0
	vcvtsi2sdq	32(%rsp), %xmm3, %xmm1
	movl	$300, %edx
	vxorpd	%xmm2, %xmm2, %xmm2
	vmovsd	.LC13(%rip), %xmm4
	vaddsd	%xmm1, %xmm0, %xmm7
	vmovsd	%xmm7, 8(%rsp)
.L12:
	xorl	%eax, %eax
	.p2align 6
	.p2align 4
	.p2align 3
.L14:
	vmulsd	(%r12,%rax), %xmm4, %xmm1
	vmovsd	0(%rbp,%rax), %xmm0
	vmulsd	(%rbx,%rax), %xmm1, %xmm1
	vfmsub132sd	%xmm0, %xmm1, %xmm0
	vucomisd	%xmm0, %xmm2
	ja	.L33
.L13:
	addq	$8, %rax
	cmpq	$80000000, %rax
	jne	.L14
	decl	%edx
	jne	.L12
	xorl	%esi, %esi
	leaq	32(%rsp), %rdi
	call	gettimeofday
	vxorps	%xmm3, %xmm3, %xmm3
	vcvtsi2sdq	40(%rsp), %xmm3, %xmm0
	vdivsd	.LC12(%rip), %xmm0, %xmm0
	vcvtsi2sdq	32(%rsp), %xmm3, %xmm3
	movl	$.LC16, %edi
	movl	$1, %eax
	vaddsd	%xmm3, %xmm0, %xmm0
	vsubsd	8(%rsp), %xmm0, %xmm0
	call	printf
	vmovsd	(%rsp), %xmm0
	movl	$.LC17, %edi
	movl	$1, %eax
	call	printf
	movq	%r15, %rdi
	call	free
	movq	%r14, %rdi
	call	free
	movq	%r13, %rdi
	call	free
	movq	%r12, %rdi
	call	free
	movq	%rbp, %rdi
	call	free
	movq	%rbx, %rdi
	call	free
	addq	$56, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
.L33:
	.cfi_restore_state
	movl	%edx, 28(%rsp)
	movq	%rax, 16(%rsp)
	call	sqrt
	vmovsd	.LC13(%rip), %xmm4
	vxorpd	%xmm2, %xmm2, %xmm2
	movl	28(%rsp), %edx
	movq	16(%rsp), %rax
	jmp	.L13
.L32:
	movl	%edx, 16(%rsp)
	movq	%rax, 8(%rsp)
	call	sqrtf
	vmovsd	.LC13(%rip), %xmm4
	vxorps	%xmm3, %xmm3, %xmm3
	vxorps	%xmm5, %xmm5, %xmm5
	movl	16(%rsp), %edx
	movq	8(%rsp), %rax
	jmp	.L9
	.cfi_endproc
.LFE22:
	.size	main, .-main
	.text
	.p2align 4
	.globl	dwalltime
	.type	dwalltime, @function
dwalltime:
.LFB23:
	.cfi_startproc
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	xorl	%esi, %esi
	movq	%rsp, %rdi
	call	gettimeofday
	vxorps	%xmm1, %xmm1, %xmm1
	vcvtsi2sdq	8(%rsp), %xmm1, %xmm0
	vdivsd	.LC12(%rip), %xmm0, %xmm0
	vcvtsi2sdq	(%rsp), %xmm1, %xmm1
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	vaddsd	%xmm1, %xmm0, %xmm0
	ret
	.cfi_endproc
.LFE23:
	.size	dwalltime, .-dwalltime
	.section	.rodata.cst4,"aM",@progbits,4
	.align 4
.LC1:
	.long	1065353216
	.align 4
.LC3:
	.long	-1065353216
	.align 4
.LC5:
	.long	1082130432
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC7:
	.long	0
	.long	1072693248
	.align 8
.LC9:
	.long	0
	.long	-1072693248
	.align 8
.LC11:
	.long	-225179981
	.long	1074790399
	.align 8
.LC12:
	.long	0
	.long	1093567616
	.align 8
.LC13:
	.long	0
	.long	1074790400
	.ident	"GCC: (GNU) 15.2.1 20260123 (Red Hat 15.2.1-7)"
	.section	.note.GNU-stack,"",@progbits
