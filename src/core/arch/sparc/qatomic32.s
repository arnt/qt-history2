	.section ".data"
	.align 4
Ddata.data:
TASI_LOCK:
	.byte 0
	.type TASI_LOCK,#object
	.size TASI_LOCK,1
TASP_LOCK:
	.byte 0
	.type TASP_LOCK,#object
	.size TASP_LOCK,1
INC_LOCK:
	.byte 0
	.type INC_LOCK,#object
	.size INC_LOCK,1
DEC_LOCK:
	.byte 0
	.type DEC_LOCK,#object
	.size DEC_LOCK,1




	.section ".text"

	.align 4
	.type q_atomic_lock,#function
	.local q_atomic_lock
q_atomic_lock:
.q_atomic_lock_try:
	ldstub [%o0],%l0
	cmp %l0,0
	be .q_atomic_lock_done
	nop
.q_atomic_lock_spin:
	ldub [%o0],%l0
	cmp %l0,0
	bne .q_atomic_lock_spin
	nop
	ba .q_atomic_lock_try
	nop
.q_atomic_lock_done:
	retl
	nop
	.size q_atomic_lock,.-q_atomic_lock




	.align 4
	.type q_atomic_unlock,#function
	.local q_atomic_unlock
q_atomic_unlock:
	stbar
	retl
	stub %g0,[%o0]
	.size q_atomic_unlock,.-q_atomic_unlock




	.align 4
	.type q_atomic_test_and_set_int,#function
	.global q_atomic_test_and_set_int
q_atomic_test_and_set_int:
	save %sp,-96,%sp
	mov %o7,%l7
	sethi %hi(TASI_LOCK),%l6
.q_atomic_tasi_PIC:
	call .+8
	sethi %hi(_GLOBAL_OFFSET_TABLE_-(.q_atomic_tasi_PIC-.)),%l5
	add %l6,%lo(TASI_LOCK),%l6
	add %l5,%lo(_GLOBAL_OFFSET_TABLE_-(.q_atomic_tasi_PIC-.)),%l5
	add %l5,%o7,%l5
	mov %l7,%o7
	call q_atomic_lock
        ld [%l5+%l6],%o0
	ld [%i0],%l0
	cmp %l0,%i1
	bne .q_atomic_test_and_set_int_done
	clr %l0
	mov 1,%l0
	st %i2,[%i0]
.q_atomic_test_and_set_int_done:
	call q_atomic_unlock
	mov %l0,%i0
	ret
	restore
	.size q_atomic_test_and_set_int,.-q_atomic_test_and_set_int




	.align 4
	.type q_atomic_test_and_set_ptr,#function
	.global q_atomic_test_and_set_ptr
q_atomic_test_and_set_ptr:
	save %sp,-96,%sp
	mov %o7,%l7
	sethi %hi(TASP_LOCK),%l6
.q_atomic_tasp_PIC:
	call .+8
	sethi %hi(_GLOBAL_OFFSET_TABLE_-(.q_atomic_tasp_PIC-.)),%l5
	add %l6,%lo(TASP_LOCK),%l5
	add %l5,%lo(_GLOBAL_OFFSET_TABLE_-(.q_atomic_tasp_PIC-.)),%l5
	add %l5,%o7,%l5
	mov %l7,%o7
	call q_atomic_lock
	ld [%l5+%l6],%o0
	ld [%i0],%l0
	cmp %l0,%i1
	bne .q_atomic_test_and_set_ptr_done
	clr %l0
	mov 1,%l0
	st %i2,[%i0]
.q_atomic_test_and_set_ptr_done:
	call q_atomic_unlock
	mov %l0,%i0
	ret
	restore
	.size q_atomic_test_and_set_ptr,.-q_atomic_test_and_set_ptr




	.align 4
	.type q_atomic_increment,#function
	.global q_atomic_increment
q_atomic_increment:
	save %sp,-96,%sp
	mov %o7,%l7
	sethi %hi(INC_LOCK),%l6
.q_atomic_increment_PIC:
	call .+8
	sethi %hi(_GLOBAL_OFFSET_TABLE_-(.q_atomic_increment_PIC-.)),%l5
	add %l6,%lo(INC_LOCK),%l6
	add %l5,%lo(_GLOBAL_OFFSET_TABLE_-(.q_atomic_increment_PIC-.)),%l5
	add %l5,%o7,%l5
	mov %l7,%o7
	call q_atomic_lock
	ld [%l5+%l6],%o0
	ld [%i0],%l0
	add %l0,1,%l0
	st %l0,[%i0]
	call q_atomic_unlock
	mov %l0,%i0
	ret
	restore
	.size q_atomic_increment,.-q_atomic_increment




	.align 4
	.type q_atomic_decrement,#function
	.global q_atomic_decrement
q_atomic_decrement:
	save %sp,-96,%sp
	mov %o7,%l7
        sethi %hi(DEC_LOCK),%l6
.q_atomic_decrement_PIC:
        call .+8
        sethi %hi(_GLOBAL_OFFSET_TABLE_-(.q_atomic_decrement_PIC-.)),%l5
        add %l6,%lo(INC_LOCK),%l6
        add %l5,%lo(_GLOBAL_OFFSET_TABLE_-(.q_atomic_decrement_PIC-.)),%l5
        add %l5,%o7,%l5
	mov %l7,%o7
	call q_atomic_lock
	ld [%l5+%l6],%o0
	ld [%i0],%l0
	add %l0,-1,%l0
	st %l0,[%i0]
	call q_atomic_unlock
	mov %l0,%i0
	ret
	restore
	.size q_atomic_decrement,.-q_atomic_decrement




	.align 4
	.type q_atomic_set_int,#function
	.global q_atomic_set_int
q_atomic_set_int:
	swap [%o0],%o1
	retl
	mov %o1,%o0
	.size q_atomic_set_int,.-q_atomic_set_int




	.align 4
	.type q_atomic_set_ptr,#function
	.global q_atomic_set_ptr
q_atomic_set_ptr:
	swap [%o0],%o1
	retl
	mov %o1,%o0
	.size q_atomic_set_ptr,.-q_atomic_set_ptr

