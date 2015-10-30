/*s: include/a.out.h */
typedef	struct	Exec	Exec;
typedef	struct	Sym	Sym;

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

    long	_unused;
    // see a.out.h man page explaining how to compute the line of a PC
    long	pcsz;		/* size of pc/line number table */
};
/*e: struct Exec */

/*s: constant HDR_MAGIC */
#define HDR_MAGIC	0x00008000		/* header expansion */
/*e: constant HDR_MAGIC */

/*s: function _MAGIC */
#define	_MAGIC(f, b)	((f)|((((4*(b))+0)*(b))+7))
/*e: function _MAGIC */

/*s: constant I_MAGIC */
#define	I_MAGIC		_MAGIC(0, 11)		/* intel 386 */
/*e: constant I_MAGIC */
/*s: constant E_MAGIC */
#define	E_MAGIC		_MAGIC(0, 20)		/* arm */
/*e: constant E_MAGIC */

/*s: constant MIN_MAGIC */
#define	MIN_MAGIC	11
/*e: constant MIN_MAGIC */
/*s: constant MAX_MAGIC */
#define	MAX_MAGIC	20			/* <= 90 */
/*e: constant MAX_MAGIC */

/*s: constant DYN_MAGIC */
#define	DYN_MAGIC	0x80000000		/* dlm */
/*e: constant DYN_MAGIC */

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
