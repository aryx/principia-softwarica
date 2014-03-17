
extern Lock	edftestlock;	/* for atomic admitting/expelling */

#pragma	varargck	type	"t"		long
#pragma	varargck	type	"U"		uvlong

/* Interface: */
Edf*		edflock(Proc*);
void		edfunlock(void);
