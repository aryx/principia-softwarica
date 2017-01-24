/*s: portfns_init.h */

// rebootcmd.c
void    rebootcmd(int, char**);

//in <arch>/main.c (but called from syswrite)
void    arch_reboot(kern_addr3, kern_addr3, ulong);

//TODO: should add prefix arch_ (or conf_ ?)
// in <arch>/<conf>.c (called from main)
void  links(void);
// in <arch>/<conf>.root.c (called from main)
void    bootlinks(void);

// in <arch>/?? (called from main) but different prototypes
//void  arch_touser(void*);

/*e: portfns_init.h */
