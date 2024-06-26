/*s: include/core/ctype.h */
#pragma	src	"/sys/src/libc/port"
#pragma	lib	"libc.a"

// could be merged in libc.h

/*s: type [[Ctype_flag]] */
#define	_U	01 // upper
#define	_L	02 // lower
#define	_N	04 // number
#define	_S	010 // space
#define	_P	020 // punctuation
#define	_C	040 // ctlr
#define	_B	0100 // ??
#define	_X	0200 // ??
/*e: type [[Ctype_flag]] */

extern unsigned char	_ctype[];

/*s: macros isxxx */
#define	isalpha(c)	(_ctype[(unsigned char)(c)]&(_U|_L))
#define	isupper(c)	(_ctype[(unsigned char)(c)]&_U)
#define	islower(c)	(_ctype[(unsigned char)(c)]&_L)
#define	isdigit(c)	(_ctype[(unsigned char)(c)]&_N)
#define	isxdigit(c)	(_ctype[(unsigned char)(c)]&_X)
#define	isspace(c)	(_ctype[(unsigned char)(c)]&_S)
#define	ispunct(c)	(_ctype[(unsigned char)(c)]&_P)
#define	isalnum(c)	(_ctype[(unsigned char)(c)]&(_U|_L|_N))
#define	isprint(c)	(_ctype[(unsigned char)(c)]&(_P|_U|_L|_N|_B))
#define	isgraph(c)	(_ctype[(unsigned char)(c)]&(_P|_U|_L|_N))
#define	iscntrl(c)	(_ctype[(unsigned char)(c)]&_C)
#define	isascii(c)	((unsigned char)(c)<=0177)
/*e: macros isxxx */
/*s: macro [[_toupper]] */
#define	_toupper(c)	((c)-'a'+'A')
/*e: macro [[_toupper]] */
/*s: macro [[_tolower]] */
#define	_tolower(c)	((c)-'A'+'a')
/*e: macro [[_tolower]] */
/*s: macro [[toascii]] */
#define	toascii(c)	((c)&0177)
/*e: macro [[toascii]] */
/*e: include/core/ctype.h */
