/*s: time/386/dat_time.h */

// Used only in 386/, so could be put in arch/ but used by the .c here.
// Actually used only in i8253.c but important so put here.

/*s: struct I8253(x86) */
struct I8253
{
    ulong   period;     /* current clock period */
    bool    enabled;

    uvlong  hz;

    ushort  last;       /* last value of clock 1 */
    uvlong  ticks;      /* cumulative ticks of counter 1 */

    ulong   periodset;

    // extra
    Lock;
};
/*e: struct I8253(x86) */

//IMPORTANT: I8253 i8253; (in i8253.c)
//IMPORTANT: also is interrupt i8253clock() calling clock.c:timerintr()
/*e: time/386/dat_time.h */
