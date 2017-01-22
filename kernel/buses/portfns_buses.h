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

/*e: portfns_buses.h */
