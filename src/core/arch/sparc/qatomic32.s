        .section        ".bss",#alloc,#write
Bbss.bss:
        .local  lock
        .common lock,4,4
        .type   lock,#object

	.section ".text"
	.type q_atomic_test_and_set_int,#function
	.global q_atomic_test_and_set_int
q_atomic_test_and_set_int:
q_atomic_test_and_set_int_spin:
        sethi %hi(lock),%l0
        or %l0,%lo(lock),%l0

        ld [%l0],%l1
        cmp %l1,0
        bne q_atomic_test_and_set_int_spin
        nop

        mov 1,%l1
        swap [%l0],%l1
        cmp %l1,1
        be q_atomic_test_and_set_int_spin
        nop

        ld [%o0],%l1
	cmp %l1,%o1
        bne q_atomic_test_and_set_int_done
        mov 0,%l1
        mov 1,%l1
	st %o2,[%o0]

q_atomic_test_and_set_int_done:
	stbar
        st %g0,[%l0]
        retl
	mov %l1,%o0
	.size q_atomic_test_and_set_int,.-q_atomic_test_and_set_int

	.type q_atomic_test_and_set_ptr,#function
	.global q_atomic_test_and_set_ptr
q_atomic_test_and_set_ptr:
q_atomic_test_and_set_ptr_spin:
        sethi %hi(lock),%l0
        or %l0,%lo(lock),%l0

	ld [%l0],%l1
	cmp %l1,0
	bne q_atomic_test_and_set_ptr_spin
	nop

	mov 1,%l1
	swap [%l0],%l1
	cmp %l1,1
	be q_atomic_test_and_set_ptr_spin
	nop

        ld [%o0],%l1
	cmp %l1,%o1
	bne q_atomic_test_and_set_ptr_done
        mov 0,%l1
        mov 1,%l1
        st %o2,[%o0]

q_atomic_test_and_set_ptr_done:
	stbar
	st %g0,[%l0]
	retl
        mov %l1,%o0
	.size q_atomic_test_and_set_ptr,.-q_atomic_test_and_set_ptr

