//@Scheck: def can actually be found initcode.c, but this file is in skip list
extern void startboot(char*, char**);

//Scheck: TODO who calls that??
void
_main(char *argv0)
{
	startboot(argv0, &argv0);
}
