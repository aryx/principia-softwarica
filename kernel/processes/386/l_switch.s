/*s: l_switch.s */

// This file is used for task switching but also to emulate
// exceptions in C (via the waserror(), nexterror(), poperror() functions)

/*s: function gotolabel */
/*
 *  label consists of a stack pointer and a PC
 */
TEXT gotolabel(SB), $0
        MOVL    label+0(FP), AX
        MOVL    0(AX), SP                       /* restore sp */
        MOVL    4(AX), AX                       /* put return pc on the stack */
        MOVL    AX, 0(SP)
        MOVL    $1, AX                          /* return true */
        RET
/*e: function gotolabel */

/*s: function setlabel */
TEXT setlabel(SB), $0
        MOVL    label+0(FP), AX
        MOVL    SP, 0(AX)                       /* store sp */
        MOVL    0(SP), BX                       /* store return pc */
        MOVL    BX, 4(AX)
        MOVL    $0, AX                          /* return false */
        RET

/*e: function setlabel */
/*e: l_switch.s */
