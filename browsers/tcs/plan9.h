/*s: tcs/plan9.h */
typedef unsigned long Rune;     /* 21 bits */
typedef unsigned char uchar;
/*s: constant [[Runeerror]](tcs) */
#define     Runeerror   0x80    /* decoding error in UTF */
/*e: constant [[Runeerror]](tcs) */
/*s: constant [[Runeself]](tcs) */
#define     Runeself    0x80    /* rune and UTF sequences are the same (<) */
/*e: constant [[Runeself]](tcs) */
/*s: constant [[UTFmax]](tcs) */
#define     UTFmax      6   /* maximum bytes per rune */
/*e: constant [[UTFmax]](tcs) */

/*
    plan 9 argument parsing
*/
/*s: constant [[ARGBEGIN]](tcs) */
#define ARGBEGIN    for((argv0? 0: (argv0= *argv)),argv++,argc--;\
                argv[0] && argv[0][0]=='-' && argv[0][1];\
                argc--, argv++) {\
                char *_args, *_argt, _argc;\
                _args = &argv[0][1];\
                if(_args[0]=='-' && _args[1]==0){\
                    argc--; argv++; break;\
                }\
                _argc=0;while(*_args) switch(_argc= *_args++)
/*e: constant [[ARGBEGIN]](tcs) */
/*s: constant [[ARGEND]](tcs) */
#define ARGEND      }
/*e: constant [[ARGEND]](tcs) */
/*s: macro [[ARGF]](tcs) */
#define ARGF()      (_argt=_args, _args="",\
                (*_argt? _argt: argv[1]? (argc--, *++argv): 0))
/*e: macro [[ARGF]](tcs) */
/*s: macro [[ARGC]](tcs) */
#define ARGC()      _argc
/*e: macro [[ARGC]](tcs) */
extern char *argv0;
/*e: tcs/plan9.h */
