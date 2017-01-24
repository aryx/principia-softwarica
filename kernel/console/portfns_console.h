/*s: portfns_console.h */

// devcons.c
// in portfns_core.h, to remove some backward dependencies
//int  (*pprint)(char*, ...);
//void (*_assert)(char*);
void kbdqinit(void);
void lineqinit(void);
void kbdputc(Rune);
int  consactive(void);
int  kbdcr2nl(Queue*, int);
int  nrand(int);
void putstrn(char*, int);
//     (*print) is declared lib.h
//int  (*iprint)(char*, ...);
//void (*panic)(char*, ...);
// as a pointer in the original too
void  (*screenputs)(char*, int);
// overrides also sysfatal from libc/9sys/sysfatal.c, that are called
// from a few libc functions

// rdb.c
void    rdb(void);

// print.c
// overrides _fmtlock, from lib_core/libc/fmt/fmtlock.c that are used
// in fmt related functions
/*e: portfns_console.h */
