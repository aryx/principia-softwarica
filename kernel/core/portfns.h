
#include "../port/portfns_core.h"
#include "../port/portfns_concurrency.h"
#include "../port/portfns_memory.h"
#include "../port/portfns_files.h"
#include "../port/portfns_processes.h"
#include "../port/portfns_misc.h"
#include "../port/portfns_console.h"
#include "../port/portfns_buses.h"
#include "../port/portfns_devices.h"
#include "../port/portfns_security.h"
#include "../port/portfns_network.h"

//in init/386/main.c (but used in port), TODO should be in portfns_init.h
void		reboot(void*, void*, ulong);

//unused and undefined:
//void		addwatchdog(Watchdog*);
//long		clrfpintr(void);
//int		decrypt(void*, void*, int);
//int		encrypt(void*, void*, int);
//int		eqchan(Chan*, Chan*, int);
//void		logopen(Log*);
//void		logclose(Log*);
//char*		logctl(Log*, int, char**, Logflag*);
//void		logn(Log*, int, void*, int);
//long		logread(Log*, void*, ulong, long);
//void		log(Log*, int, char*, ...);
//void		resched(char*);
//void		savefpregs(FPsave*);

#pragma varargck argpos iprint	1
#pragma varargck argpos pprint	1
#pragma	varargck argpos	panic	1
