/*s: portfns_buses.h */

// devuart.c
int   uartstageoutput(Uart*);
void  uartkick(void*);
int   uartgetc(void);
//void    uartputc(int);
void  uartputs(char*, int);
void  uartrecv(Uart*, char);
int   uartctl(Uart*, char*);
//void    uartmouse(Uart*, int (*)(Queue*, int), int);
//void    uartsetmouseputc(Uart*, int (*)(Queue*, int));

// TODO: move outside main.c?
// <arch>/main.c for now (called from port)
int arch_isaconfig(char*, int, ISAConf*);

/*e: portfns_buses.h */
