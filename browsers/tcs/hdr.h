/*s: tcs/hdr.h */
extern int squawk;
extern int clean;
extern char *file;
extern int verbose;
extern long ninput, noutput, nrunes, nerrors;

enum { From = 1, Table = 2, Func = 4 };

typedef void (*Fnptr)(void);
struct convert{
    char *name;
    char *chatter;
    int flags;
    void *data;
    Fnptr fn;
};
extern struct convert convert[];
struct convert *conv(char *, int);
typedef void (*Infn)(int, long *, struct convert *);
typedef void (*Outfn)(Rune *, int, long *);
void outtable(Rune *, int, long *);

void utf_in(int, long *, struct convert *);
void utf_out(Rune *, int, long *);
void isoutf_in(int, long *, struct convert *);
void isoutf_out(Rune *, int, long *);

/*s: constant [[N]](tcs) */
#define     N       10000       /* just blocking */
/*e: constant [[N]](tcs) */
/*s: macro [[OUT]](tcs) */
#define OUT(out, r, n)  if(out->flags&Table) outtable(r, n, (long *)out->data);\
            else ((Outfn)(out->fn))(r, n, (long *)0)
/*e: macro [[OUT]](tcs) */

extern Rune runes[N];
extern char obuf[UTFmax*N]; /* maximum bloat from N runes */

/*s: constant [[BADMAP]](tcs) */
#define     BADMAP      (0xFFFD)
/*e: constant [[BADMAP]](tcs) */
/*s: constant [[BYTEBADMAP]](tcs) */
#define     BYTEBADMAP  ('?')       /* badmap but has to fit in a byte */
/*e: constant [[BYTEBADMAP]](tcs) */
/*s: constant [[ESC]](tcs) */
#define     ESC     033
/*e: constant [[ESC]](tcs) */

#ifdef  PLAN9
/*s: constant [[EPR]](tcs) */
#define EPR     fprint(2,
/*e: constant [[EPR]](tcs) */
/*s: macro [[EXIT]](tcs) */
#define EXIT(n,s)   exits(s)
/*e: macro [[EXIT]](tcs) */
#else
/*s: constant [[EPR (tcs/hdr.h)]](tcs) */
#define EPR     fprintf(stderr,
/*e: constant [[EPR (tcs/hdr.h)]](tcs) */
/*s: macro [[USED]](tcs) */
#define USED(x)     /* in plan 9, USED(x) tells the compiler to treat x as used */
/*e: macro [[USED]](tcs) */
/*s: macro [[EXIT (tcs/hdr.h)]](tcs) */
#define EXIT(n,s)   exit(n)
/*e: macro [[EXIT (tcs/hdr.h)]](tcs) */
#endif
/*e: tcs/hdr.h */
