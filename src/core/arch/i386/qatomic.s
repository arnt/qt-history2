	.text
	.align 4,0x90
	.globl q_atomic_test_and_set_int
q_atomic_test_and_set_int:
	movl		 4(%esp),%ecx
	movl		 8(%esp),%eax
	movl		12(%esp),%edx
	lock cmpxchgl	%edx,(%ecx)
	mov		$0,%eax
 	sete		%al
	ret
	.align 4,0x90
	.type q_atomic_test_and_set_int,@function
	.size q_atomic_test_and_set_int,.-q_atomic_test_and_set_int
	.align 4,0x90
	.globl q_atomic_test_and_set_ptr
q_atomic_test_and_set_ptr:
	movl		 4(%esp),%ecx
	movl		 8(%esp),%eax
	movl		12(%esp),%edx
	lock cmpxchgl	%edx,(%ecx)
	mov		$0,%eax
	sete		%al
	ret
	.align    4,0x90
	.type	q_atomic_test_and_set_ptr,@function
	.size	q_atomic_test_and_set_ptr,.-q_atomic_test_and_set_ptr
