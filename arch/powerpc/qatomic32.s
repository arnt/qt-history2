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
	lwarx 0,0,3
	cmpw 0,4
	bne- $+12
	stwcx. 5,0,3
	bne- $-16
	mr 3,0
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
	.globl q_cas_ptr
	.globl .q_cas_ptr
	.csect q_cas_ptr[DS]
q_cas_ptr:
	.long .q_cas_ptr, TOC[tc0], 0
	.csect .text[PR]
.q_cas_ptr:
	lwarx 0,0,3
	cmpw 0,4
	bne- $+12
	stwcx. 5,0,3
	bne- $-16
	mr 3,0
	blr
LT..q_cas_ptr:
	.long 0
	.byte 0,9,32,96,128,1,3,1
	.long 0
	.long LT..q_cas_ptr-.q_cas_ptr
	.short 9
	.byte "q_cas_ptr"
	.byte 31

