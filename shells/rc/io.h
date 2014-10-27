/*s: rc/io.h */
/*s: constant EOF (rc/io.h) */
#define	EOF	(-1)
/*e: constant EOF (rc/io.h) */
/*s: constant NBUF */
#define	NBUF	512
/*e: constant NBUF */

/*s: struct io */
struct Io {
    fdt	fd;
    byte	*bufp, *ebuf, *strp;
    byte	buf[NBUF];
};
/*e: struct io */

extern io *err;

io *openfd(int);
io *openstr(void);
io *opencore(char *, int);
void pchr(io*, int);
int rchr(io*);
int rutf(io*, char*, Rune*);
void closeio(io*);
void flush(io*);
void pstr(io*, char*);
void pcmd(io*, tree*);
void pfnc(io*, thread*);
void pfmt(io*, char*, ...);
/*e: rc/io.h */
