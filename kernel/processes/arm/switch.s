/*s: processes/arm/switch.s */
/*s: function [[arch_setlabel]](arm) */
TEXT arch_setlabel(SB), 1, $-4
    MOVW    R13, 0(R0)      /* sp */
    MOVW    R14, 4(R0)      /* pc */
    MOVW    $0, R0
    RET
/*e: function [[arch_setlabel]](arm) */

/*s: function [[arch_gotolabel]](arm) */
TEXT arch_gotolabel(SB), 1, $-4
    MOVW    0(R0), R13      /* sp */
    MOVW    4(R0), R14      /* pc */
    MOVW    $1, R0
    RET
/*e: function [[arch_gotolabel]](arm) */

/*s: function [[getcallerpc]](arm) */
TEXT getcallerpc(SB), 1, $-4
    MOVW    0(R13), R0
    RET
/*e: function [[getcallerpc]](arm) */

/*e: processes/arm/switch.s */
