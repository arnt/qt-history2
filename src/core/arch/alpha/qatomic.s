	.set volatile
	.set noat
	.arch ev4
	.text
	.globl q_cas_32
	.ent q_cas_32
q_cas_32:
	.frame $30,0,$26,0
	.prologue 0
1:	ldl_l $2,0($16)
	cmpeq $2,$17,$1
	beq $1,4f
	mov $18,$1
	stl_c $1,0($16)
	beq $1,3f
2:	br 4f
3:	br 1b
4:	addl $31,$2,$0
	ret $31,($26),1
	.end q_cas_32
	.globl q_cas_ptr
	.ent q_cas_ptr
q_cas_ptr:
	.frame $30,0,$26,0
	.prologue 0
1:	ldq_l $0,0($16)
	cmpeq $0,$17,$1
	beq $1,4f
	mov $18,$1
	stq_c $1,0($16)
	beq $1,3f
2:	br 4f
3:	br 1b
4:	ret $31,($26),1
	.end q_cas_ptr
