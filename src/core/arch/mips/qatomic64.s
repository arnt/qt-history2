	.text
	.globl	q_cas_32
	.ent	q_cas_32
q_cas_32:
	.frame	$sp,0,$31
1:	ll   $2,0($4)
	bne  $2,$5,2f
	move $3,$0
	move $3,$6
	sc   $3,0($4)
	beqz $3,1b
2:	j	$31
	.end	q_cas_32

	.globl	q_cas_ptr
	.ent	q_cas_ptr
q_cas_ptr:
	.frame	$sp,0,$31
1:	lld  $2,0($4)
	bne  $2,$5,2f
	move $3,$0
	move $3,$6
	scd  $3,0($4)
	beqz $3,1b
2:	j	$31
	.end	q_cas_ptr
