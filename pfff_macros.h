
// include/libc.h
#define ARGBEGIN switch(1) 
#define ARGEND 
#define EXTERN extern
#define Extern extern

#define va_start(list, start) list =\
	(sizeof(start) < 4?\
		(char*)((int*)&(start)+1):\
		(char*)(&(start)+1))

#define va_end(list)\
	USED(list)

#define va_arg(list, mode)\
	((sizeof(mode) == 1)?\
		((list += 4), (mode*)list)[-4]:\
	(sizeof(mode) == 2)?\
		((list += 4), (mode*)list)[-2]:\
		((list += sizeof(mode)), (mode*)list)[-1])

#define offsetof(s, m)  (ulong)(&(((s*)0)->m))


// generators/yacc/yacc.c
// TODO: could improve loop heuristic finder in pp_hacks?
#define TLOOP(i)	for(i=1; i<=ntokens; i++)
#define NTLOOP(i)	for(i=0; i<=nnonter; i++)
#define PLOOP(s,i)	for(i=s; i<nprod; i++)
#define SLOOP(i)	for(i=0; i<nstate; i++)
#define WSBUMP(x)	x++
#define WSLOOP(s,j)	for(j=s; j<cwp; j++)
#define ITMLOOP(i,p,q)	for(q=pstate[i+1], p=pstate[i]; p<q; p++)
#define SETLOOP(i)	for(i=0; i<tbitset; i++)
