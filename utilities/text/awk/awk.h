/*s: awk/awk.h */
/*
Copyright (c) Lucent Technologies 1997
    All Rights Reserved

*/

typedef double  Awkfloat;

/* unsigned char is more trouble than it's worth */

typedef unsigned char uschar;

#define xfree(a)    { if ((a) != NULL) { free((char *) a); a = NULL; } }

#define DEBUG
#ifdef  DEBUG
            /* uses have to be doubly parenthesized */
#   define  dprintf(x)  if (dbg) printf x
#else
#   define  dprintf(x)
#endif

extern  char    errbuf[];

extern int  compile_time;   /* 1 if compiling, 0 if running */
extern int  safe;       /* 0 => unsafe, 1 => safe */

#define RECSIZE (8 * 1024)  /* sets limit on records, fields, etc., etc. */
extern int  recsize;    /* size of current record, orig RECSIZE */

extern char **FS;
extern char **RS;
extern char **ORS;
extern char **OFS;
extern char **OFMT;
extern Awkfloat *NR;
extern Awkfloat *FNR;
extern Awkfloat *NF;
extern char **FILENAME;
extern char **SUBSEP;
extern Awkfloat *RSTART;
extern Awkfloat *RLENGTH;

extern char *record;    /* points to $0 */
extern int  lineno;     /* line number in awk program */
extern int  errorflag;  /* 1 if error has occurred */
extern int  donefld;    /* 1 if record broken into fields */
extern int  donerec;    /* 1 if record is valid (no fld has changed */
extern char inputFS[];  /* FS at time of input, for field splitting */

extern int  dbg;

extern  char    *patbeg;    /* beginning of pattern matched */
extern  int patlen;     /* length of pattern matched.  set in b.c */

/* Cell:  all information about a variable or constant */

typedef struct Cell {
    uschar  ctype;      /* OCELL, OBOOL, OJUMP, etc. */
    uschar  csub;       /* CCON, CTEMP, CFLD, etc. */
    char    *nval;      /* name, for variables only */
    char    *sval;      /* string value */
    Awkfloat fval;      /* value as number */
    int  tval;      /* type info: STR|NUM|ARR|FCN|FLD|CON|DONTFREE */
    struct Cell *cnext; /* ptr to next if chained */
} Cell;

typedef struct Array {      /* symbol table array */
    int nelem;      /* elements in table right now */
    int size;       /* size of tab */
    Cell    **tab;      /* hash table pointers */
} Array;

#define NSYMTAB 50  /* initial size of a symbol table */
extern Array    *symtab;

extern Cell *nrloc;     /* NR */
extern Cell *fnrloc;    /* FNR */
extern Cell *nfloc;     /* NF */
extern Cell *rstartloc; /* RSTART */
extern Cell *rlengthloc;    /* RLENGTH */

/* Cell.tval values: */
#define NUM 01  /* number value is valid */
#define STR 02  /* string value is valid */
#define DONTFREE 04 /* string space is not freeable */
#define CON 010 /* this is a constant */
#define ARR 020 /* this is an array */
#define FCN 040 /* this is a function name */
#define FLD 0100    /* this is a field $1, $2, ... */
#define REC 0200    /* this is $0 */


/* function types */
#define FLENGTH 1
#define FSQRT   2
#define FEXP    3
#define FLOG    4
#define FINT    5
#define FSYSTEM 6
#define FRAND   7
#define FSRAND  8
#define FSIN    9
#define FCOS    10
#define FATAN   11
#define FTOUPPER 12
#define FTOLOWER 13
#define FFLUSH  14
#define FUTF    15

/* Node:  parse tree is made of nodes, with Cell's at bottom */

typedef struct Node {
    int ntype;
    struct  Node *nnext;
    int lineno;
    int nobj;
    struct  Node *narg[1];  /* variable: actual size set by calling malloc */
} Node;

#define NIL ((Node *) 0)

extern Node *winner;
extern Node *nullstat;
extern Node *nullnode;

/* ctypes */
#define OCELL   1
#define OBOOL   2
#define OJUMP   3

/* Cell subtypes: csub */
#define CFREE   7
#define CCOPY   6
#define CCON    5
#define CTEMP   4
#define CNAME   3 
#define CVAR    2
#define CFLD    1
#define CUNK    0

/* bool subtypes */
#define BTRUE   11
#define BFALSE  12

/* jump subtypes */
#define JEXIT   21
#define JNEXT   22
#define JBREAK  23
#define JCONT   24
#define JRET    25
#define JNEXTFILE   26

/* node types */
#define NVALUE  1
#define NSTAT   2
#define NEXPR   3


extern  int pairstack[], paircnt;

#define notlegal(n) (n <= FIRSTTOKEN || n >= LASTTOKEN || proctab[n-FIRSTTOKEN] == nullproc)
#define isvalue(n)  ((n)->ntype == NVALUE)
#define isexpr(n)   ((n)->ntype == NEXPR)
#define isjump(n)   ((n)->ctype == OJUMP)
#define isexit(n)   ((n)->csub == JEXIT)
#define isbreak(n)  ((n)->csub == JBREAK)
#define iscont(n)   ((n)->csub == JCONT)
#define isnext(n)   ((n)->csub == JNEXT)
#define isnextfile(n)   ((n)->csub == JNEXTFILE)
#define isret(n)    ((n)->csub == JRET)
#define isrec(n)    ((n)->tval & REC)
#define isfld(n)    ((n)->tval & FLD)
#define isstr(n)    ((n)->tval & STR)
#define isnum(n)    ((n)->tval & NUM)
#define isarr(n)    ((n)->tval & ARR)
#define isfcn(n)    ((n)->tval & FCN)
#define istrue(n)   ((n)->csub == BTRUE)
#define istemp(n)   ((n)->csub == CTEMP)
#define isargument(n)   ((n)->nobj == ARG)
/* #define freeable(p)  (!((p)->tval & DONTFREE)) */
#define freeable(p) ( ((p)->tval & (STR|DONTFREE)) == STR )

#include "proto.h"
/*e: awk/awk.h */
