	.file	"qatomic.cpp"
	.toc
	.csect .text[PR]
	.align 2
	.globl q_cas_32
	.globl .q_cas_32
	.csect q_cas_32[DS]
q_cas_32:
	.long .q_cas_32, TOC[tc0], 0
	.csect .text[PR]
.q_cas_32:
LFB..40:
	stw 31,-4(1)
LCFI..0:
	stwu 1,-48(1)
LCFI..1:
	mr 31,1
LCFI..2:
	stw 3,72(31)
	stw 4,76(31)
	stw 5,80(31)
	lwz 10,72(31)
	lwz 11,72(31)
	lwz 9,76(31)
	lwz 0,80(31)
	
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
	stw 0,28(31)
	addi 9,31,28
	lwz 0,0(9)
	stw 0,24(31)
	lwz 0,24(31)
	mr 3,0
	lwz 1,0(1)
	lwz 31,-4(1)
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
	.csect q_cas_ptr[DS]
q_cas_ptr:
	.long .q_cas_ptr, TOC[tc0], 0
	.csect .text[PR]
.q_cas_ptr:
LFB..42:
	stw 31,-4(1)
LCFI..3:
	stwu 1,-48(1)
LCFI..4:
	mr 31,1
LCFI..5:
	stw 3,72(31)
	stw 4,76(31)
	stw 5,80(31)
	lwz 10,72(31)
	lwz 11,72(31)
	lwz 9,76(31)
	lwz 0,80(31)
	
q_cas_ptr_retry:
	lwarx 8, 0, 11
	cmplw 9, 8
	bne q_cas_ptr_store
	stwcx. 0, 0, 11
	bne- q_cas_ptr_retry
	b q_cas_ptr_out
q_cas_ptr_store:
	stwcx. 8, 0, 11
q_cas_ptr_out:

	mr 0,8
	stw 0,28(31)
	addi 9,31,28
	lwz 0,0(9)
	stw 0,24(31)
	lwz 0,24(31)
	mr 3,0
	lwz 1,0(1)
	lwz 31,-4(1)
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
	.long _section_.text
