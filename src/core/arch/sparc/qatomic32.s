.section ".text"
.align 4
.type q_atomic_test_and_set_int,#function
.global q_atomic_test_and_set_int
q_atomic_test_and_set_int:
q_atomic_test_and_set_int_spin:
    ld [%o0],%l0
    cmp %l0,-1
    be q_atomic_test_and_set_int_spin

    mov -1,%l0
    swap [%o0],%l0
    cmp %l0,-1
    be q_atomic_test_and_set_int_spin

    cmp %l0,%o1
    bne q_atomic_test_and_set_int_failed
    stbar
    st %o2,[%o0]
    retl
    mov %l0,%o0
q_atomic_test_and_set_int_failed:
    stbar
    st %l0,[%o0]
    retl
    mov %l0,%o0
.size q_atomic_test_and_set_int,.-q_atomic_test_and_set_int
.align 4
.type q_atomic_test_and_set_ptr,#function
.global q_atomic_test_and_set_ptr
q_atomic_test_and_set_ptr:
q_atomic_test_and_set_ptr_spin:
    ld [%o0],%l0
    cmp %l0,-1
    be q_atomic_test_and_set_ptr_spin
    mov -1,%l0
    swap [%o0],%l0
    cmp %l0,-1
    be q_atomic_test_and_set_ptr_spin
    cmp %l0,%o1
    bne q_atomic_test_and_set_ptr_failed
    stbar
    st %o2,[%o0]
    retl
    mov %l0,%o0
q_atomic_test_and_set_ptr_failed:
    stbar
    st %l0,[%o0]
    retl
    mov %l0,%o0
.size q_atomic_test_and_set_ptr,.-q_atomic_test_and_set_ptr
