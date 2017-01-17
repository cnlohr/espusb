
//Shamelessly ripped from esp-open-rtos
.section .UserEnter.text
        .org    0


VecBase:
        .global VecBase
        /* IMPORTANT: exception vector literals will go here, but we
         * can't have more than 4 otherwise we push DebugExceptionVector past
         * offset 0x10 relative to VecBase. There should be ways to avoid this,
         * and also keep the VecBase offsets easy to read, but this works for
         * now. */
        .literal_position

CallUserStart:
		j main

        .org    VecBase + 0x10
DebugExceptionVector:
        .type   DebugExceptionVector, @function
		.byte 00  //illegal instruction

        .org    VecBase + 0x20
NMIExceptionVector:
        .type   NMIExceptionVector, @function
        j   NMIExceptionHandler

        .org    VecBase + 0x30
KernelExceptionVector:
        .type   KernelExceptionVector, @function

		.byte 00  //illegal instruction

/*        break   1, 0
        mov     a2, a1
        movi    a3, 0
        call0   fatal_exception_handler
        rfe*/

        .org    VecBase + 0x50
UserExceptionVector:
        .type   UserExceptionVector, @function

//        wsr     a1, excsave1
//        rsr     a1, exccause
        j       UserExceptionHandler

        .org    VecBase + 0x70
DoubleExceptionVector:
        .type   DoubleExceptionVector, @function
		.byte 00  //illegal instruction


