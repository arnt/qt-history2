	.toc
	.csect .text[PR]
	.align 2
	.globl q_atomic_test_and_set_int
	.globl .q_atomic_test_and_set_int
	.csect q_atomic_test_and_set_int[DS]
q_atomic_test_and_set_int:
	.long .q_atomic_test_and_set_int, TOC[tc0], 0
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
	mr 3,9
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
	.csect q_atomic_test_and_set_ptr[DS]
q_atomic_test_and_set_ptr:
	.long .q_atomic_test_and_set_ptr, TOC[tc0], 0
	.csect .text[PR]
.q_atomic_test_and_set_ptr:
	lwarx  0,0,3
	cmpw   0,4
	bne-   $+20
	stwcx.  5,0,3
	bne-   $+12
	li     9,1
	b      $+8
	li     9,0
	mr 3,9
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
	.long _section_.text
