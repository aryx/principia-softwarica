/*s: rc/io.h */
/*s: constant EOF (rc/io.h) */
#define	EOF	(-1)
/*e: constant EOF (rc/io.h) */
/*s: constant [[NBUF]] */
#define	NBUF	512
/*e: constant [[NBUF]] */

/*s: struct [[Io]] */
struct Io {
    fdt	fd;
    byte	*bufp, *ebuf, *strp;
    byte	buf[NBUF];
};
/*e: struct [[Io]] */

extern io *err;

// io.c
io *openfd(fdt);
io *openstr(void);
io *opencore(char *, int);
void closeio(io*);
void flush(io*);

int rchr(io*);
int rutf(io*, char*, Rune*);

// fmt.c
void pchr(io*, int);
void pstr(io*, char*);
void pfmt(io*, char*, ...);

// pcmd.c
void pcmd(io*, tree*);
// pfnc.c
void pfnc(io*, thread*);
/*e: rc/io.h */
