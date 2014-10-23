/*s: rc/io.h */
/*s: constant EOF (rc/io.h) */
#define	EOF	(-1)
/*e: constant EOF (rc/io.h) */
/*s: constant NBUF */
#define	NBUF	512
/*e: constant NBUF */

/*s: struct io */
struct Io {
    int	fd;
    uchar	*bufp, *ebuf, *strp;
    uchar	buf[NBUF];
};
/*e: struct io */
/*s: global err */
io *err;
/*e: global err */

io *openfd(int), *openstr(void), *opencore(char *, int);
int emptybuf(io*);
void pchr(io*, int);
int rchr(io*);
int rutf(io*, char*, Rune*);
void closeio(io*);
void flush(io*);
int fullbuf(io*, int);
void pdec(io*, int);
void poct(io*, unsigned);
void pptr(io*, void*);
void pquo(io*, char*);
void pwrd(io*, char*);
void pstr(io*, char*);
void pcmd(io*, tree*);
void pval(io*, word*);
void pfnc(io*, thread*);
void pfmt(io*, char*, ...);
/*e: rc/io.h */
