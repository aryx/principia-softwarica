// from include/386/u.h
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


// from include/libc.h
#define	ARGBEGIN	for((argv0||(argv0=*argv)),argv++,argc--;\
			    argv[0] && argv[0][0]=='-' && argv[0][1];\
			    argc--, argv++) {\
				char *_args, *_argt;\
				Rune _argc;\
				_args = &argv[0][1];\
				if(_args[0]=='-' && _args[1]==0){\
					argc--; argv++; break;\
				}\
				_argc = 0;\
				while(*_args && (_args += chartorune(&_argc, _args)))\
				switch(_argc)
#define	ARGEND		SET(_argt);USED(_argt,_argc,_args);}USED(argv, argc);

#define offsetof(s, m)  (ulong)(&(((s*)0)->m))


// misc adhoc stuff, maybe not needed anymore
#define EXTERN extern
#define Extern extern


// generators/yacc/yacc.c
// (could improve loop heuristic finder in pp_hacks?)
#define TLOOP(i)	for(i=1; i<=ntokens; i++)
#define NTLOOP(i)	for(i=0; i<=nnonter; i++)
#define PLOOP(s,i)	for(i=s; i<nprod; i++)
#define SLOOP(i)	for(i=0; i<nstate; i++)
#define WSBUMP(x)	x++
#define WSLOOP(s,j)	for(j=s; j<cwp; j++)
#define ITMLOOP(i,p,q)	for(q=pstate[i+1], p=pstate[i]; p<q; p++)
#define SETLOOP(i)	for(i=0; i<tbitset; i++)

// lib_core/libc/port/pool.c
#define antagonism	if(!(p->flags & POOL_ANTAGONISM)){}else
#define paranoia	if(!(p->flags & POOL_PARANOIA)){}else
#define verbosity	if(!(p->flags & POOL_VERBOSITY)){}else

// linkers/libmach/8.c
// otherwise get wrong dependency on the argument that looks like an entity
#define	REGOFF(x)	(ulong)(&((struct Ureg *) 0)->x)

// shells/rc/exec.c
// otherwise get wrong dependency on the argument that looks like a global
//#define	new(type)	((type *)emalloc(sizeof(type)))

