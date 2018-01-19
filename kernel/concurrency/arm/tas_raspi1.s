/*s: concurrency/arm/tas_raspi1.s */
TEXT    _tas(SB), $-4
/*s: function [[arch_tas]](raspberry pi1)(arm) */
TEXT    arch_tas(SB), $-4
    MOVW    R0,R1
    MOVW    $1,R0
    SWPW    R0,(R1)         /* fix: deprecated in armv6 */
    RET
/*e: function [[arch_tas]](raspberry pi1)(arm) */
/*e: concurrency/arm/tas_raspi1.s */
