	.section __TEXT,__text,regular,pure_instructions
	.section __TEXT,__picsymbolstub1,symbol_stubs,pure_instructions,32
        .section __TEXT,__text,regular,pure_instructions
	.align 2
	.align 2
	.globl _q_atomic_test_and_set_int
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_test_and_set_int:
	lwarx  r6,0,r3
        cmpw   r6,r4
        bne-   $+20
        stwcx. r5,0,r3
        bne-   $-16
        addi   r3,r0,1
        blr
        addi   r3,r0,0
	blr

	.align 2
	.globl _q_atomic_test_and_set_ptr
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_test_and_set_ptr:
	lwarx  r6,0,r3
        cmpw   r6,r4
        bne-   $+20
        stwcx. r5,0,r3
        bne-   $-16
        addi   r3,r0,1
        blr
        addi   r3,r0,0
	blr

	.align 2
	.globl _q_atomic_increment
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_increment:
	li r4,1
	lwarx  r4,0,r3
        add    r4,r4,r2
        stwcx. r4,0,r3
        bne-   $-12
	mr     r3,r4
	blr

	.align 2
	.globl _q_atomic_decrement
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_decrement:
	li r4,-1
	lwarx  r4,0,r3
        add    r4,r4,r2
        stwcx. r4,0,r3
        bne-   $-12
	mr     r3,r4
	blr

	.align 2
	.globl _q_atomic_set_int
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_set_int:
	lwarx  r5,0,r3
        stwcx. r4,0,r3
        bne-   $-8
	mr     r3,r5
	blr

	.align 2
	.globl _q_atomic_set_ptr
        .section __TEXT,__text,regular,pure_instructions
	.align 2
_q_atomic_set_ptr:
	lwarx  r5,0,r3
        stwcx. r4,0,r3
        bne-   $-8
	mr     r3,r5
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
