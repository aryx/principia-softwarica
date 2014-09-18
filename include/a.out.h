/*s: include/a.out.h */
/*s: struct Exec */
// a.out header format
struct	Exec
{
    long	magic;		/* magic number */

    long	text;	 	/* size of text segment */
    long	data;	 	/* size of initialized data */
    long	bss;	  	/* size of uninitialized data */

    long	syms;	 	/* size of symbol table */

    // virtual address in [UTZERO+sizeof(Exec)..UTZERO+sizeof(Exec)+text]
    long	entry;	 	/* entry point */ 

    // used only by 68020, spsize set to 0 for 8l
    long	spsz;		/* size of pc/sp offset table */
    // see a.out.h man page explaining how to compute the line of a PC
    long	pcsz;		/* size of pc/line number table */
};
/*e: struct Exec */
typedef	struct	Exec	Exec;

/*s: constant HDR_MAGIC */
#define HDR_MAGIC	0x00008000		/* header expansion */
/*e: constant HDR_MAGIC */

/*s: function _MAGIC */
#define	_MAGIC(f, b)	((f)|((((4*(b))+0)*(b))+7))
/*e: function _MAGIC */

/*s: constant A_MAGIC */
#define	A_MAGIC		_MAGIC(0, 8)		/* 68020 (retired) */
/*e: constant A_MAGIC */
/*s: constant I_MAGIC */
#define	I_MAGIC		_MAGIC(0, 11)		/* intel 386 */
/*e: constant I_MAGIC */
/*s: constant J_MAGIC */
#define	J_MAGIC		_MAGIC(0, 12)		/* intel 960 (retired) */
/*e: constant J_MAGIC */
/*s: constant K_MAGIC */
#define	K_MAGIC		_MAGIC(0, 13)		/* sparc */
/*e: constant K_MAGIC */
/*s: constant V_MAGIC */
#define	V_MAGIC		_MAGIC(0, 16)		/* mips 3000 BE */
/*e: constant V_MAGIC */
/*s: constant X_MAGIC */
#define	X_MAGIC		_MAGIC(0, 17)		/* att dsp 3210 (retired) */
/*e: constant X_MAGIC */
/*s: constant M_MAGIC */
#define	M_MAGIC		_MAGIC(0, 18)		/* mips 4000 BE */
/*e: constant M_MAGIC */
/*s: constant D_MAGIC */
#define	D_MAGIC		_MAGIC(0, 19)		/* amd 29000 (retired) */
/*e: constant D_MAGIC */
/*s: constant E_MAGIC */
#define	E_MAGIC		_MAGIC(0, 20)		/* arm */
/*e: constant E_MAGIC */
/*s: constant Q_MAGIC */
#define	Q_MAGIC		_MAGIC(0, 21)		/* powerpc */
/*e: constant Q_MAGIC */
/*s: constant N_MAGIC */
#define	N_MAGIC		_MAGIC(0, 22)		/* mips 4000 LE */
/*e: constant N_MAGIC */
/*s: constant L_MAGIC */
#define	L_MAGIC		_MAGIC(0, 23)		/* dec alpha (retired) */
/*e: constant L_MAGIC */
/*s: constant P_MAGIC */
#define	P_MAGIC		_MAGIC(0, 24)		/* mips 3000 LE */
/*e: constant P_MAGIC */
/*s: constant U_MAGIC */
#define	U_MAGIC		_MAGIC(0, 25)		/* sparc64 (retired) */
/*e: constant U_MAGIC */
/*s: constant S_MAGIC */
#define	S_MAGIC		_MAGIC(HDR_MAGIC, 26)	/* amd64 */
/*e: constant S_MAGIC */
/*s: constant T_MAGIC */
#define	T_MAGIC		_MAGIC(HDR_MAGIC, 27)	/* powerpc64 */
/*e: constant T_MAGIC */
/*s: constant R_MAGIC */
#define	R_MAGIC		_MAGIC(HDR_MAGIC, 28)	/* arm64 */
/*e: constant R_MAGIC */

/*s: constant MIN_MAGIC */
#define	MIN_MAGIC	8
/*e: constant MIN_MAGIC */
/*s: constant MAX_MAGIC */
#define	MAX_MAGIC	28			/* <= 90 */
/*e: constant MAX_MAGIC */

/*s: constant DYN_MAGIC */
#define	DYN_MAGIC	0x80000000		/* dlm */
/*e: constant DYN_MAGIC */

typedef	struct	Sym	Sym;
/*s: struct Sym a.out.h */
struct	Sym
{
    vlong	value;
    uint	sig;
    char	type;
    char	*name;
};
/*e: struct Sym a.out.h */
/*e: include/a.out.h */
