/*s: portfns_buses.h */

// devuart.c
int   uartgetc(void);
void  uartputs(char*, int);
void  uartrecv(Uart*, char);
int   uartctl(Uart*, char*);
int   uartstageoutput(Uart*);
void  uartkick(void*);

// TODO: move outside main.c?
// <arch>/main.c for now (called from port)
//int arch_isaconfig(char*, int, ISAConf*); // now in core/ for backward deps

/*e: portfns_buses.h */
