	.set noreorder
	.set volatile
	.set noat
	.arch ev4
	.text
	.align 2
	.align 4
	.globl q_atomic_test_and_set_int
	.ent q_atomic_test_and_set_int
q_atomic_test_and_set_int:
	.frame $30,0,$26,0
	.prologue 0
	ldl_l $0,0($16)
	cmpeq $0,$17,$0
	beq   $0,1f
	mov   $18,$0
	stl_c $0,0($16)
1:
	addl $31,$0,$0
	ret $31,($26),1
	.end q_atomic_test_and_set_int
	.align 2
	.align 4
	.globl q_atomic_test_and_set_ptr
	.ent q_atomic_test_and_set_ptr
q_atomic_test_and_set_ptr:
	.frame $30,0,$26,0
	.prologue 0
	ldq_l $0,0($16)
	cmpeq $0,$17,$0
	beq   $0,1f
	mov   $18,$0
	stq_c $0,0($16)
1:
	addl $31,$0,$0
	ret $31,($26),1
	.end q_atomic_test_and_set_ptr
