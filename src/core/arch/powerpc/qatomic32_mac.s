	.section __TEXT,__text,regular,pure_instructions
	.section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32
        .section __TEXT,__text,regular,pure_instructions
	.align 2
	.align 2
	.globl _q_atomic_test_and_set_int
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_test_and_set_int:
	lwarx  r0,0,r3
        cmpw   r0,r4
        bne-   $+20
        stwcx. r5,0,r3
        bne-   $-20
        li     r2,1
        b      $+8
        li     r2,0
	mr r3,r2
	blr

	.align 2
	.globl _q_atomic_test_and_set_ptr
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_test_and_set_ptr:
	lwarx  r0,0,r3
        cmpw   r0,r4
        bne-   $+20
        stwcx.  r5,0,r3
        bne-   $-20
        li     r2,1
        b      $+8
        li     r2,0
	mr r3,r2
	blr

	.align 2
	.globl _q_atomic_increment
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_increment:
	li r4,1
	lwarx  r2, 0, r3
        add    r2, r4, r2
        stwcx. r2, 0, r3
        bne-   $-12
	mr r3,r2
	blr

	.align 2
	.globl _q_atomic_decrement
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_decrement:
	li r4,-1
	lwarx  r2, 0, r3
        add    r2, r4, r2
        stwcx. r2, 0, r3
        bne-   $-12
	mr r3,r2
	blr

	.align 2
	.globl _q_atomic_set_int
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_set_int:
	lwarx  r0, 0, r3
        stwcx. r4, 0, r3
        bne-   $-8

	mr r3,r0
	blr

	.align 2
	.globl _q_atomic_set_ptr
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_set_ptr:
	lwarx  r0, 0, r3
        stwcx.  r4, 0, r3
        bne-   $-8

	mr r3,r0
	blr

.globl q_atomic_test_and_set_int.eh
	q_atomic_test_and_set_int.eh = 0
.globl q_atomic_test_and_set_ptr.eh
	q_atomic_test_and_set_ptr.eh = 0
.globl q_atomic_increment.eh
	q_atomic_increment.eh = 0
.globl q_atomic_decrement.eh
	q_atomic_decrement.eh = 0
.globl q_atomic_set_int.eh
	q_atomic_set_int.eh = 0
.globl q_atomic_set_ptr.eh
	q_atomic_set_ptr.eh = 0
.data
.constructor
.data
.destructor
.align 1
