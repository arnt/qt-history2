	.machine	"ppc64"
	.toc
	.csect .text[PR]
	.align 2
	.globl q_atomic_test_and_set_int
	.globl .q_atomic_test_and_set_int
	.csect q_atomic_test_and_set_int[DS],3
q_atomic_test_and_set_int:
	.llong .q_atomic_test_and_set_int, TOC[tc0], 0
	.csect .text[PR]
.q_atomic_test_and_set_int:
	lwarx  0,0,3
	cmpw   0,4
	bne-   $+20
	stwcx. 5,0,3
	bne-   $+12
	li     9,1
	b      $+8
	li     9,0
	extsw 3,9
	blr
LT..q_atomic_test_and_set_int:
	.long 0
	.byte 0,0,32,64,0,0,3,0
	.long 0
	.long LT..q_atomic_test_and_set_int-.q_atomic_test_and_set_int
	.short 25
	.byte "q_atomic_test_and_set_int"
	.align 2
	.align 2
	.globl q_atomic_test_and_set_ptr
	.globl .q_atomic_test_and_set_ptr
	.csect q_atomic_test_and_set_ptr[DS],3
q_atomic_test_and_set_ptr:
	.llong .q_atomic_test_and_set_ptr, TOC[tc0], 0
	.csect .text[PR]
.q_atomic_test_and_set_ptr:
	ldarx  0,0,3
	cmpd   0,4
	bne-   $+20
	stdcx.  5,0,3
	bne-   $+12
	li     9,1
	b      $+8
	li     9,0
	extsw 3,9
	blr
LT..q_atomic_test_and_set_ptr:
	.long 0
	.byte 0,0,32,64,0,0,3,0
	.long 0
	.long LT..q_atomic_test_and_set_ptr-.q_atomic_test_and_set_ptr
	.short 25
	.byte "q_atomic_test_and_set_ptr"
	.align 2
_section_.text:
	.csect .data[RW],3
	.llong _section_.text
