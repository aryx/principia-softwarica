/*s: init9.c */
//@Scheck: def can actually be found initcode.c, but this file is in skip list
extern void startboot(char*, char**);

//@Scheck: entry point looked for by the linker 8l 
void
_main(char *argv0)
{
    startboot(argv0, &argv0);
}
/*e: init9.c */
