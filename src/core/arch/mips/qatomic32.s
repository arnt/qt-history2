	.set nobopt
	.option pic2
	.text
	.globl	q_atomic_test_and_set_int
	.ent	q_atomic_test_and_set_int
q_atomic_test_and_set_int:
	.frame	$sp,0,$31
	ll   $3,0($4)
	.set noreorder
	bne  $3,$5,1f
	move $2,$0
	.set reorder
	move $2,$6
	sc   $2,0($4)
1:	j    $31
	.end	q_atomic_test_and_set_int
	.globl	q_atomic_test_and_set_ptr
	.ent	q_atomic_test_and_set_ptr
q_atomic_test_and_set_ptr:
	.frame	$sp,0,$31
	ll   $3,0($4)
	.set noreoder
	bne  $3,$5,1f
	move $2,$0
	.set reorder
	move $2,$6
	sc   $2,0($4)
1:	j    $31
	.end	q_atomic_test_and_set_ptr
