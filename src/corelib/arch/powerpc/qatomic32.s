	.machine "ppc"
	.toc
	.csect .text[PR]

	.align 2
	.globl q_atomic_test_and_set_int
	.globl .q_atomic_test_and_set_int
	.csect q_atomic_test_and_set_int[DS],3
q_atomic_test_and_set_int:
	.long .q_atomic_test_and_set_int,TOC[tc0],0
	.csect .text[PR]
.q_atomic_test_and_set_int:
	lwarx  6,0,3
	cmpw   6,4
	bne-   $+20
	stwcx. 5,0,3
	bne-   $-16
	addi   3,0,1
	blr
	addi   3,0,0
	blr
LT..q_atomic_test_and_set_int:
	.long 0
	.byte 0,9,32,64,0,0,3,0
	.long 0
	.long LT..q_atomic_test_and_set_int-.q_atomic_test_and_set_int
	.short 25
	.byte "q_atomic_test_and_set_int"
	.align 2

	.align 2
	.globl q_atomic_test_and_set_acquire_int
	.globl .q_atomic_test_and_set_acquire_int
	.csect q_atomic_test_and_set_acquire_int[DS],3
q_atomic_test_and_set_acquire_int:
	.long .q_atomic_test_and_set_acquire_int,TOC[tc0],0
	.csect .text[PR]
.q_atomic_test_and_set_acquire_int:
	lwarx  6,0,3
	cmpw   6,4
	bne-   $+20
	stwcx. 5,0,3
	bne-   $-16
	addi   3,0,1
	b      $+8
	addi   3,0,0
	eieio
	blr
LT..q_atomic_test_and_set_acquire_int:
	.long 0
	.byte 0,9,32,64,0,0,3,0
	.long 0
	.long LT..q_atomic_test_and_set_acquire_int-.q_atomic_test_and_set_acquire_int
	.short 25
	.byte "q_atomic_test_and_set_acquire_int"
	.align 2

	.align 2
	.globl q_atomic_test_and_set_release_int
	.globl .q_atomic_test_and_set_release_int
	.csect q_atomic_test_and_set_release_int[DS],3
q_atomic_test_and_set_release_int:
	.long .q_atomic_test_and_set_release_int,TOC[tc0],0
	.csect .text[PR]
.q_atomic_test_and_set_release_int:
	eieio
	lwarx  6,0,3
	cmpw   6,4
	bne-   $+20
	stwcx. 5,0,3
	bne-   $-16
	addi   3,0,1
	blr
	addi   3,0,0
	blr
LT..q_atomic_test_and_set_release_int:
	.long 0
	.byte 0,9,32,64,0,0,3,0
	.long 0
	.long LT..q_atomic_test_and_set_release_int-.q_atomic_test_and_set_release_int
	.short 25
	.byte "q_atomic_test_and_set_release_int"
	.align 2

	.align 2
	.globl q_atomic_test_and_set_ptr
	.globl .q_atomic_test_and_set_ptr
	.csect q_atomic_test_and_set_ptr[DS],3
q_atomic_test_and_set_ptr:
	.long .q_atomic_test_and_set_ptr,TOC[tc0],0
	.csect .text[PR]
.q_atomic_test_and_set_ptr:
	lwarx  6,0,3
	cmpw   6,4
	bne-   $+20
	stwcx. 5,0,3
	bne-   $-16
	addi   3,0,1
	blr
	addi   3,0,0
	blr
LT..q_atomic_test_and_set_ptr:
	.long 0
	.byte 0,9,32,64,0,0,3,0
	.long 0
	.long LT..q_atomic_test_and_set_ptr-.q_atomic_test_and_set_ptr
	.short 25
	.byte "q_atomic_test_and_set_ptr"
	.align 2

	.align 2
	.globl q_atomic_increment
	.globl .q_atomic_increment
	.csect q_atomic_increment[DS],3
q_atomic_increment:
	.long .q_atomic_increment,TOC[tc0],0
	.csect .text[PR]
.q_atomic_increment:
	lwarx  4,0,3
	addi   4,4,1
	stwcx. 4,0,3
	bne-   $-12
	mr     3,4
	blr
LT..q_atomic_increment:
	.long 0
	.byte 0,9,32,64,0,0,1,0
	.long 0
	.long LT..q_atomic_increment-.q_atomic_increment
	.short 18
	.byte "q_atomic_increment"
	.align 2

	.align 2
	.globl q_atomic_decrement
	.globl .q_atomic_decrement
	.csect q_atomic_decrement[DS],3
q_atomic_decrement:
	.long .q_atomic_decrement,TOC[tc0],0
	.csect .text[PR]
.q_atomic_decrement:
	lwarx  4,0,3
	subi   4,4,1
	stwcx. 4,0,3
	bne-   $-12
	mr     3,4
	blr
LT..q_atomic_decrement:
	.long 0
	.byte 0,9,32,64,0,0,1,0
	.long 0
	.long LT..q_atomic_decrement-.q_atomic_decrement
	.short 18
	.byte "q_atomic_decrement"
	.align 2

	.align 2
	.globl q_atomic_set_int
	.globl .q_atomic_set_int
	.csect q_atomic_set_int[DS],3
q_atomic_set_int:
	.long .q_atomic_set_int,TOC[tc0],0
	.csect .text[PR]
.q_atomic_set_int:
	lwarx  5,0,3
	stwcx. 4,0,3
	bne-   $-8
	mr     3,5
	blr
LT..q_atomic_set_int:
	.long 0
	.byte 0,9,32,64,0,0,2,0
	.long 0
	.long LT..q_atomic_set_int-.q_atomic_set_int
	.short 16
	.byte "q_atomic_set_int"
	.align 2

	.align 2
	.globl q_atomic_set_ptr
	.globl .q_atomic_set_ptr
	.csect q_atomic_set_ptr[DS],3
q_atomic_set_ptr:
	.long .q_atomic_set_ptr,TOC[tc0],0
	.csect .text[PR]
.q_atomic_set_ptr:
	lwarx  5,0,3
	stwcx. 4,0,3
	bne-   $-8
	mr     3,5
	blr
LT..q_atomic_set_ptr:
	.long 0
	.byte 0,9,32,64,0,0,2,0
	.long 0
	.long LT..q_atomic_set_ptr-.q_atomic_set_ptr
	.short 16
	.byte "q_atomic_set_ptr"
	.align 2

_section_.text:
	.csect .data[RW],3
	.long _section_.text

