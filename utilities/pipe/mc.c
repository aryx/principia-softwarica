/*s: pipe/mc.c */
/*
 * mc - columnate
 *
 * mc[-][-LINEWIDTH][-t][file...]
 *  - causes break on colon
 *  -LINEWIDTH sets width of line in which to columnate(default 80)
 *  -t suppresses expanding multiple blanks into tabs
 *
 */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include    <bio.h>

#ifdef Unix
char* font;
#else
#include    <draw.h>
Font *font;
#endif

/*s: constants mc.c */
#define WIDTH           80
#define TAB 4
/*x: constants mc.c */
#define ALLOC_QUANTA        4096
#define WORD_ALLOC_QUANTA   1024
/*e: constants mc.c */
/*s: global flags mc.c */
// ??
bool colonflag = false;
/*x: global flags mc.c */
// set to true in acme
bool tabflag = false;  /* -t flag turned off forever */
/*e: global flags mc.c */
/*s: globals mc.c */
int linewidth=WIDTH;
/*x: globals mc.c */
Biobuf  bin;
Biobuf  bout;
/*x: globals mc.c */
Rune *cbuf, *cbufp;
Rune **word;
/*x: globals mc.c */
int nalloc=ALLOC_QUANTA;
int nchars=0;
/*x: globals mc.c */
int nwords=0;
int mintab=1;
int maxwidth=0;
/*x: globals mc.c */
int nwalloc=WORD_ALLOC_QUANTA;
/*x: globals mc.c */
int tabwidth=0;
/*e: globals mc.c */

// forward decls
void getwidth(void), readbuf(int), error(char *);
void scanwords(void), columnate(void), morechars(void);
int wordwidth(Rune*, int);
int nexttab(int);

/*s: function [[main]](mc.c) */
void
main(int argc, char *argv[])
{
    int i;
    bool lineset = false;
    fdt ifd;

    Binit(&bout, STDOUT, OWRITE);

    // Why no ARGBEGIN/ARGEND?
    while(argc > 1 && argv[1][0] == '-'){
        --argc; argv++;
        switch(argv[0][1]){
        case '\0':
            colonflag = true;
            break;
        // useful only for acme
        case 't':
            tabflag = false;
            break;
        default:
            linewidth = atoi(&argv[0][1]);
            if(linewidth <= 1)
                linewidth = WIDTH;
            lineset = true;
            break;
        }
    }

    if(!lineset){
        getwidth();
        if(linewidth <= 1){
            linewidth = WIDTH;
            font = nil;
        }
    }

    cbuf = cbufp = malloc(ALLOC_QUANTA*(sizeof *cbuf));
    word = malloc(WORD_ALLOC_QUANTA*(sizeof *word));
    if(word == nil || cbuf == nil)
        error("out of memory");

    if(argc == 1)
        readbuf(STDIN);
    else{
        for(i = 1; i < argc; i++){
            if((ifd = open(*++argv, OREAD)) == -1)
                fprint(STDERR, "mc: can't open %s (%r)\n", *argv);
            else{
                readbuf(ifd);
                Bflush(&bin);
                close(ifd);
            }
        }
    }
    columnate();
    exits(nil);
}
/*e: function [[main]](mc.c) */

/*s: function [[error]](mc.c) */
void
error(char *s)
{
    fprint(STDERR, "mc: %s\n", s);
    exits(s);
}
/*e: function [[error]](mc.c) */
/*s: function [[readbuf]](mc.c) */
void
readbuf(fdt fd)
{
    int lastwascolon = 0;
    int linesiz = 0;
    long c;

    Binit(&bin, fd, OREAD);
    do{
        if(nchars++ >= nalloc)
            morechars();
        *cbufp++ = c = Bgetrune(&bin);
        linesiz++;
        if(c == '\t') {
            cbufp[-1] = L' ';
            while(linesiz%TAB != 0) {
                if(nchars++ >= nalloc)
                    morechars();
                *cbufp++ = L' ';
                linesiz++;
            }
        }
        if(colonflag && c == ':')
            lastwascolon++;
        else if(lastwascolon){
            if(c == '\n'){
                --nchars;   /* skip newline */
                *cbufp = L'\0';
                while(nchars > 0 && cbuf[--nchars] != '\n')
                    ;
                if(nchars)
                    nchars++;
                columnate();
                if (nchars)
                    Bputc(&bout, '\n');
                Bprint(&bout, "%S", cbuf+nchars);
                nchars = 0;
                cbufp = cbuf;
            }
            lastwascolon = 0;
        }
        if(c == '\n')
            linesiz = 0;
    }while(c >= 0);
}
/*e: function [[readbuf]](mc.c) */
/*s: function [[scanwords]](mc.c) */
void
scanwords(void)
{
    Rune *p, *q;
    int i, w;

    nwords=0;
    maxwidth=0;
    for(p = q = cbuf, i = 0; i < nchars; i++){
        if(*p++ == L'\n'){
            if(nwords >= nwalloc){
                nwalloc += WORD_ALLOC_QUANTA;
                if((word = realloc(word, nwalloc*sizeof(*word)))==0)
                    error("out of memory");
            }
            word[nwords++] = q;
            p[-1] = L'\0';
            w = wordwidth(q, p-q-1);
            if(w > maxwidth)
                maxwidth = w;
            q = p;
        }
    }
}
/*e: function [[scanwords]](mc.c) */
/*s: function [[columnate]](mc.c) */
void
columnate(void)
{
    int i, j;
    int words_per_line;
    int nlines;
    int col;
    int endcol;


    scanwords();
    if(nwords==0)
        return;
    maxwidth = nexttab(maxwidth+mintab-1);
    words_per_line = linewidth/maxwidth;
    if(words_per_line <= 0)
        words_per_line = 1;
    nlines=(nwords+words_per_line-1)/words_per_line;
    for(i = 0; i < nlines; i++){
        col = endcol = 0;
        for(j = i; j < nwords; j += nlines){
            endcol += maxwidth;
            Bprint(&bout, "%S", word[j]);
            col += wordwidth(word[j], runestrlen(word[j]));
            if(j+nlines < nwords){
                if(tabflag) {
                    while(col < endcol){
                        Bputc(&bout, '\t');
                        col = nexttab(col);
                    }
                }else{
                    while(col < endcol){
                        Bputc(&bout, ' ');
                        col++;
                    }
                }
            }
        }
        Bputc(&bout, '\n');
    }
}
/*e: function [[columnate]](mc.c) */
/*s: function [[nexttab]](mc.c) */
int
nexttab(int col)
{
    if(tabwidth){
        col += tabwidth;
        col -= col%tabwidth;
        return col;
    }
    return col+1;
}
/*e: function [[nexttab]](mc.c) */
/*s: function [[morechars]](mc.c) */
void
morechars(void)
{
    nalloc += ALLOC_QUANTA;
    if((cbuf = realloc(cbuf, nalloc*sizeof(*cbuf))) == nil)
        error("out of memory");
    cbufp = cbuf+nchars-1;
}
/*e: function [[morechars]](mc.c) */

#ifdef Unix
/*s: function [[getwidth]](mc.c)(unix) */
void getwidth(void)
{
}
/*e: function [[getwidth]](mc.c)(unix) */
/*s: function [[wordwidth]](mc.c)(unix) */
int
wordwidth(Rune *w, int nw)
{
    return nw;
}
/*e: function [[wordwidth]](mc.c)(unix) */
#else
int
wordwidth(Rune *w, int nw)
{
    if(font)
        return runestringnwidth(font, w, nw);
    return nw;
}

/*
 * These routines discover the width of the display.
 * It takes some work.  If we do the easy calls to the
 * draw library, the screen flashes due to repainting
 * when mc exits.
 */

jmp_buf drawjmp;

void
terror(Display*, char*)
{
    longjmp(drawjmp, 1);
}

void
getwidth(void)
{
    int n, fd;
    char buf[128], *f[10], *p;

    if(access("/dev/acme", OREAD) >= 0){
        if((fd = open("/dev/acme/ctl", OREAD)) < 0)
            return;
        n = read(fd, buf, sizeof buf-1);
        close(fd);
        if(n <= 0)
            return;
        buf[n] = 0;
        n = tokenize(buf, f, nelem(f));
        if(n < 7)
            return;
        if((font = openfont(nil, f[6])) == nil)
            return;
        if(n >= 8)
            tabwidth = atoi(f[7]);
        else
            tabwidth = 4*stringwidth(font, "0");
        mintab = stringwidth(font, "0");
        linewidth = atoi(f[5]);
        tabflag = 1;
        return;
    }

    if((p = getenv("font")) == nil)
        return;
    if((font = openfont(nil, p)) == nil)
        return;
    if((fd = open("/dev/window", OREAD)) < 0){
        font = nil;
        return;
    }
    n = read(fd, buf, 5*12);
    close(fd);
    if(n < 5*12){
        font = nil;
        return;
    }
    buf[n] = 0;
    
    /* window stucture:
        4 bit left edge
        1 bit gap
        12 bit scrollbar
        4 bit gap
        text
        4 bit right edge
    */
    linewidth = atoi(buf+3*12) - atoi(buf+1*12) - (4+1+12+4+4);
    mintab = stringwidth(font, "0");
    if((p = getenv("tabstop")) != nil)
        tabwidth = atoi(p)*stringwidth(font, "0");
    if(tabwidth == 0)
        tabwidth = 4*stringwidth(font, "0");
    tabflag = 1;
}
#endif
/*e: pipe/mc.c */
