	.section ".text"
	.align 4
	.type q_atomic_test_and_set_int,#function
	.global q_atomic_test_and_set_int
q_atomic_test_and_set_int:
	cas [%o0],%o1,%o2
	cmp %o1,%o2
        clr %o0
	retl
	move %icc,1,%o0
	.size q_atomic_test_and_set_int,.-q_atomic_test_and_set_int
	.align 4
	.type q_atomic_test_and_set_ptr,#function
	.global q_atomic_test_and_set_ptr
q_atomic_test_and_set_ptr:
	casx [%o0],%o1,%o2
	cmp %o1,%o2
	clr %o0
	retl
	move %icc,1,%o0
	.size q_atomic_test_and_set_ptr,.-q_atomic_test_and_set_ptr
	.align 4

