	.file	"qatomic.cpp"
	.machine	"ppc64"
	.toc
	.csect .text[PR]
	.align 2
	.globl q_cas_32
	.globl .q_cas_32
	.csect q_cas_32[DS],3
q_cas_32:
	.llong .q_cas_32, TOC[tc0], 0
	.csect .text[PR]
.q_cas_32:
LFB..40:
	std 31,-8(1)
LCFI..0:
	stdu 1,-80(1)
LCFI..1:
	mr 31,1
LCFI..2:
	std 3,128(31)
	mr 0,4
	mr 9,5
	stw 0,136(31)
	mr 0,9
	stw 0,144(31)
	ld 10,128(31)
	ld 11,128(31)
	lwz 9,136(31)
	lwz 0,144(31)
	
q_cas_32_retry:
	lwarx 8, 0, 11
	cmplw 9, 8
	bne q_cas_32_store
	stwcx. 0, 0, 11
	bne- q_cas_32_retry
	b q_cas_32_out
q_cas_32_store:
	stwcx. 8, 0, 11
q_cas_32_out:

	mr 0,8
	stw 0,52(31)
	addi 9,31,52
	lwz 0,0(9)
	stw 0,48(31)
	lwz 0,48(31)
	extsw 0,0
	mr 3,0
	ld 1,0(1)
	ld 31,-8(1)
	blr
LT..q_cas_32:
	.long 0
	.byte 0,9,32,96,128,1,3,1
	.long 0
	.long LT..q_cas_32-.q_cas_32
	.short 8
	.byte "q_cas_32"
	.byte 31
	.align 2
LFE..40:
	.align 2
	.globl q_cas_ptr
	.globl .q_cas_ptr
	.csect q_cas_ptr[DS],3
q_cas_ptr:
	.llong .q_cas_ptr, TOC[tc0], 0
	.csect .text[PR]
.q_cas_ptr:
LFB..42:
	std 31,-8(1)
LCFI..3:
	stdu 1,-80(1)
LCFI..4:
	mr 31,1
LCFI..5:
	std 3,128(31)
	std 4,136(31)
	std 5,144(31)
	ld 10,128(31)
	ld 11,128(31)
	ld 9,136(31)
	ld 0,144(31)
	
q_cas_ptr_retry:
	ldarx 8, 0, 11
	cmpld 9, 8
	bne q_cas_ptr_store
	stdcx. 0, 0, 11
	bne- q_cas_ptr_retry
	b q_cas_ptr_out
q_cas_ptr_store:
	stdcx. 8, 0, 11
q_cas_ptr_out:

	mr 0,8
	std 0,56(31)
	addi 9,31,56
	lfd 0,0(9)
	stfd 0,48(31)
	ld 0,48(31)
	mr 3,0
	ld 1,0(1)
	ld 31,-8(1)
	blr
LT..q_cas_ptr:
	.long 0
	.byte 0,9,32,96,128,1,3,1
	.long 0
	.long LT..q_cas_ptr-.q_cas_ptr
	.short 9
	.byte "q_cas_ptr"
	.byte 31
	.align 2
LFE..42:
_section_.text:
	.csect .data[RW],3
	.llong _section_.text
