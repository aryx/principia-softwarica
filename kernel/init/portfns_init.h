/*s: portfns_init.h */

//in init/<arch>/main.c (but called from syswrite)
void    arch_reboot(kern_addr3, kern_addr3, ulong);

// rebootcmd.c
//void    readn(Chan *, void *, long);
void    rebootcmd(int, char**);

/*e: portfns_init.h */
