/*s: networking/ip/snoopy/dat.h */
typedef struct Field Field;
typedef struct Filter Filter;
typedef struct Msg Msg;
typedef struct Mux Mux;
typedef struct Proto Proto;

/*s: macro NetS (networking/ip/snoopy/dat.h) */
#define NetS(x) ((((uchar*)x)[0]<<8) | ((uchar*)x)[1])
/*e: macro NetS (networking/ip/snoopy/dat.h) */
/*s: macro Net3 */
#define Net3(x) ((((uchar*)x)[0]<<16) | (((uchar*)x)[1]<<8) | ((uchar*)x)[2])
/*e: macro Net3 */
/*s: macro NetL (networking/ip/snoopy/dat.h) */
#define NetL(x) ((((uchar*)x)[0]<<24) | (((uchar*)x)[1]<<16) | (((uchar*)x)[2]<<8) | ((uchar*)x)[3])
/*e: macro NetL (networking/ip/snoopy/dat.h) */

/*s: struct Proto (networking/ip/snoopy/dat.h) */
/*
 *  one per protocol module
 */
struct Proto
{
    char*	name;
    void	(*compile)(Filter*);
    int	(*filter)(Filter*, Msg*);
    int	(*seprint)(Msg*);
    Mux*	mux;
    char*	valfmt;
    Field*	field;
    int	(*framer)(int, uchar*, int);
};
/*e: struct Proto (networking/ip/snoopy/dat.h) */
extern Proto *protos[];

/*s: struct Mux */
/*
 *  one per protocol module, pointed to by Proto.mux
 */
struct Mux
{
    char*	name;
    ulong	val;
    Proto*	pr;
};
/*e: struct Mux */

/*s: struct Field */
/*
 *  a field defining a comparison filter
 */
struct Field
{
    char*	name;
    int	ftype;
    int	subop;
    char*	help;
};
/*e: struct Field */

/*s: struct Msg (networking/ip/snoopy/dat.h) */
/*
 *  the status of the current message walk
 */
struct Msg
{
    uchar	*ps;	/* packet ptr */
    uchar	*pe;	/* packet end */

    char	*p;	/* buffer start */
    char	*e;	/* buffer end */

    int	needroot;	/* pr is root, need to see in expression */
    Proto	*pr;	/* current/next protocol */	
};
/*e: struct Msg (networking/ip/snoopy/dat.h) */

/*s: enum _anon_ (networking/ip/snoopy/dat.h) */
enum
{
    Fnum,		/* just a number */
    Fether,		/* ethernet address */
    Fv4ip,		/* v4 ip address */
    Fv6ip,		/* v6 ip address */
    Fba,		/* byte array */
};
/*e: enum _anon_ (networking/ip/snoopy/dat.h) */

/*s: struct Filter */
/*
 *  a node in the filter tree
 */
struct Filter {
    int	op;	/* token type */
    char	*s;	/* string */
    Filter	*l;
    Filter	*r;

    Proto	*pr;	/* next protocol;

    /* protocol specific */
    int	subop;
    ulong	param;
    union {
        ulong	ulv;
        vlong	vlv;
        uchar	a[32];
    };
};
/*e: struct Filter */

extern void	yyinit(char*);
extern int	yyparse(void);

extern Filter*	newfilter(void);
extern void	compile_cmp(char*, Filter*, Field*);
extern void	demux(Mux*, ulong, ulong, Msg*, Proto*);
extern int	defaultframer(int, uchar*, int);

extern int Mflag;
extern int Nflag;
extern int dflag;
extern int Cflag;

typedef Filter *Filterptr;
/*s: constant YYSTYPE */
#define YYSTYPE Filterptr
/*e: constant YYSTYPE */
extern Filter *filter;
/*e: networking/ip/snoopy/dat.h */
