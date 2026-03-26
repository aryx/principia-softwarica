/*s: libc/arm/main9.s */
/*s: constant NPRIVATES(arm) */
#define NPRIVATES       16
/*e: constant NPRIVATES(arm) */
/*s: constant NOPROF(arm) */
#define NOPROF 1 // see 5.out.h
/*e: constant NOPROF(arm) */
arg=0
sp=13
sb=12

TEXT    _main(SB), NOPROF, $(16 + NPRIVATES*4)
        MOVW    $setR12(SB), R(sb)
        MOVW    R(arg), _tos(SB)

        /*s: [[_main()]](arm) setup private storage */
        MOVW    $p-64(SP), R1
        MOVW    R1, _privates(SB)
        MOVW    $NPRIVATES, R1
        MOVW    R1, _nprivates(SB)
        /*e: [[_main()]](arm) setup private storage */

        MOVW    $inargv+0(FP), R(arg)
        MOVW    R(arg), 8(R(sp))
        MOVW    inargc-4(FP), R(arg)
        MOVW    R(arg), 4(R(sp))

        // user main()
        BL      main(SB)

loop:
        /*s: [[_main()]](arm) exit (if not done by main) */
        MOVW    $_exitstr<>(SB), R(arg)
        MOVW    R(arg), 4(R(sp))
        BL      exits(SB)
        // unreachable
        // force loading of div?
        BL      _div(SB)
        B       loop
        /*e: [[_main()]](arm) exit (if not done by main) */

/*s: [[_main()]](arm) [[_exitstr]] defintion */
DATA    _exitstr<>+0(SB)/4, $"main"
GLOBL   _exitstr<>+0(SB), $5
/*e: [[_main()]](arm) [[_exitstr]] defintion */
/*e: libc/arm/main9.s */
