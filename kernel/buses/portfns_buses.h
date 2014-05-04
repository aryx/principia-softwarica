/*s: portfns_buses.h */

// devuart.c
void    uartsetmouseputc(Uart*, int (*)(Queue*, int));
int   uartstageoutput(Uart*);
void    uartkick(void*);
int   uartgetc(void);
//void    uartputc(int);
void    uartputs(char*, int);
void    uartrecv(Uart*, char);
int   uartctl(Uart*, char*);
void    uartmouse(Uart*, int (*)(Queue*, int), int);

// devpnp.c
// dead?
/*e: portfns_buses.h */
