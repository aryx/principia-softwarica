/*s: init9.c */
//@Scheck: def can actually be found initcode.c, but this file is in skip list
extern void startboot(char*, char**);

//@Scheck: entry point looked for by the linker 8l 
//For x86, no need for an init9.s; there is no special registers
//to set in assembly (for example for arm we need to set R12 and we can
//do that only from assembly).
void
_main(char *argv0)
{
    startboot(argv0, &argv0);
}
/*e: init9.c */
