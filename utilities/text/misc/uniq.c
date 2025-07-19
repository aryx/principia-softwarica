/*s: pipe/uniq.c */
/*
 * Deal with duplicated lines in a file
 */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>
#include <ctype.h>

/*s: constant [[SIZE]](uniq.c) */
#define SIZE    8000
/*e: constant [[SIZE]](uniq.c) */

/*s: globals uniq.c */
int fields  = 0;
int letters = 0;
// 'u' or 'd' or 'c' or 's' (meaning??)
char    mode;
/*x: globals uniq.c */
int linec   = 0;
/*x: globals uniq.c */
bool uniq;
/*x: globals uniq.c */
char    *b1, *b2;
long    bsize = SIZE;
/*x: globals uniq.c */
Biobuf  fin;
Biobuf  fout;
/*e: globals uniq.c */

// forward decls
bool   gline(char *buf);
void  pline(char *buf);
bool   equal(char *b1, char *b2);
char* skip(char *s);

/*s: function [[main]](uniq.c) */
void
main(int argc, char *argv[])
{
    fdt f = STDIN;

    argv0 = argv[0]; // use??
    b1 = malloc(bsize);
    b2 = malloc(bsize);
    while(argc > 1) {
        if(*argv[1] == '-') {
            if(isdigit(argv[1][1]))
                fields = atoi(&argv[1][1]);
            else
                mode = argv[1][1];
            argc--;
            argv++;
            continue;
        }
        if(*argv[1] == '+') {
            letters = atoi(&argv[1][1]);
            argc--;
            argv++;
            continue;
        }
        f = open(argv[1], OREAD);
        if(f < 0)
            sysfatal("cannot open %s", argv[1]);
        break;
    }
    if(argc > 2)
        sysfatal("unexpected argument %s", argv[2]);
    Binit(&fin, f, OREAD);
    Binit(&fout, STDOUT, OWRITE);

    if(gline(b1))
        exits(nil);

    for(;;) {
        linec++;
        if(gline(b2)) {
            pline(b1);
            exits(nil);
        }
        if(!equal(b1, b2)) {
            pline(b1);
            linec = 0;
            do {
                linec++;
                if(gline(b1)) {
                    pline(b2);
                    exits(nil);
                }
            } while(equal(b2, b1));
            pline(b2);
            linec = 0;
        }
    }
}
/*e: function [[main]](uniq.c) */

/*s: function [[gline]](uniq.c) */
bool
gline(char *buf)
{
    int len;
    char *p;

    p = Brdline(&fin, '\n');
    if(p == nil)
        return true;
    len = Blinelen(&fin);
    if(len >= bsize-1)
        sysfatal("line too long");
    memmove(buf, p, len);
    buf[len-1] = '\0';
    return false;
}
/*e: function [[gline]](uniq.c) */
/*s: function [[pline]](uniq.c) */
void
pline(char *buf)
{
    switch(mode) {

    case 'u':
        if(uniq) {
            uniq = false;
            return;
        }
        break;

    case 'd':
        if(uniq)
            break;
        return;

    case 'c':
        Bprint(&fout, "%4d ", linec);
    }
    uniq = false;
    Bprint(&fout, "%s\n", buf);
}
/*e: function [[pline]](uniq.c) */
/*s: function [[equal]](uniq.c) */
bool
equal(char *b1, char *b2)
{
    char c;

    if(fields || letters) {
        b1 = skip(b1);
        b2 = skip(b2);
    }
    for(;;) {
        c = *b1++;
        if(c != *b2++) {
            if(c == '\0' && mode == 's')
                return true;
            return false;
        }
        if(c == '\0') {
            uniq = true;
            return true;
        }
    }
}
/*e: function [[equal]](uniq.c) */
/*s: function [[skip]](uniq.c) */
char*
skip(char *s)
{
    int nf, nl;

    nf = nl = 0;
    while(nf++ < fields) {
        while(*s == ' ' || *s == '\t')
            s++;
        while(!(*s == ' ' || *s == '\t' || *s == '\0') ) 
            s++;
    }
    while(nl++ < letters && *s != '\0') 
            s++;
    return s;
}
/*e: function [[skip]](uniq.c) */
/*e: pipe/uniq.c */
