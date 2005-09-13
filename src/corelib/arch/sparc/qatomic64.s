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
	.type q_atomic_test_and_set_acquire_int,#function
	.global q_atomic_test_and_set_acquire_int
q_atomic_test_and_set_acquire_int:
	cas [%o0],%o1,%o2
	cmp %o1,%o2
	clr %o0
	membar #LoadLoad | #LoadStore
	retl
	move %icc,1,%o0
	.size q_atomic_test_and_set_acquire_int,.-q_atomic_test_and_set_acquire_int

	.align 4
	.type q_atomic_test_and_set_release_int,#function
	.global q_atomic_test_and_set_release_int
q_atomic_test_and_set_release_int:
        membar #LoadStore | #StoreStore
	cas [%o0],%o1,%o2
	cmp %o1,%o2
	clr %o0
	retl
	move %icc,1,%o0
	.size q_atomic_test_and_set_release_int,.-q_atomic_test_and_set_release_int

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
	.type q_atomic_increment,#function
	.global q_atomic_increment
q_atomic_increment:
q_atomic_increment_retry:
	ld [%o0],%o3
	add %o3,1,%o4
	cas [%o0],%o3,%o4
	cmp %o3,%o4
	bne q_atomic_increment_retry
	nop
	cmp %o4,-1
	clr %o0
	retl
	movne %icc,1,%o0
	.size q_atomic_increment,.-q_atomic_increment

	.align 4
	.type q_atomic_decrement,#function
	.global q_atomic_decrement
q_atomic_decrement:
q_atomic_decrement_retry:
	ld [%o0],%o3
	add %o3,-1,%o4
	cas [%o0],%o3,%o4
	cmp %o3,%o4
	bne q_atomic_decrement_retry
	nop
	cmp %o4,1
	clr %o0
	retl
	movne %icc,1,%o0
	.size q_atomic_decrement,.-q_atomic_decrement

	.align 4
	.type q_atomic_set_int,#function
	.global q_atomic_set_int
q_atomic_set_int:
q_atomic_set_int_retry:
	ld [%o0],%o2
	cas [%o0],%o2,%o1
	cmp %o2,%o1
	bne q_atomic_set_int_retry
	nop
	retl
	mov %o1,%o0
	.size q_atomic_set_int,.-q_atomic_set_int

	.align 4
	.type q_atomic_set_ptr,#function
	.global q_atomic_set_ptr
q_atomic_set_ptr:
q_atomic_set_ptr_retry:
	ldx [%o0],%o2
	casx [%o0],%o2,%o1
	cmp %o2,%o1
	bne q_atomic_set_ptr_retry
	nop
	retl
	mov %o1,%o0
	.size q_atomic_set_ptr,.-q_atomic_set_ptr

