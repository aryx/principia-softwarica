/*s: portfns_init.h */

//in init/386/main.c (but used in port)
void    reboot(void*, void*, ulong);

// rebootcmd.c
//void    readn(Chan *, void *, long);
void    rebootcmd(int, char**);

/*e: portfns_init.h */
