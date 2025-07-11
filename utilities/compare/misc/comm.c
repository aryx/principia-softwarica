/*s: compare/comm.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>

/*s: constant [[LB]](comm.c) */
#define LB 2048
/*e: constant [[LB]](comm.c) */
/*s: global flags comm.c */
bool one;
bool two;
bool three;
/*e: global flags comm.c */
/*s: globals comm.c */
char    *ldr[3];
/*x: globals comm.c */
Biobuf *ib1;
Biobuf *ib2;
/*e: globals comm.c */

// forward decls
Biobuf* openfil(char*);
int     rd(Biobuf*, char*);
void    wr(char*, int);
void    copy(Biobuf*, char*, int);
int     compare(char*, char*);

/*s: function [[main]](comm.c) */
void
main(int argc, char *argv[])
{
    int l;
    char    lb1[LB],lb2[LB];

    ldr[0] = "";
    ldr[1] = "\t";
    ldr[2] = "\t\t";
    l = 1;

    ARGBEGIN{
    case '1':
        if(!one) {
            one = true;
            ldr[1][0] = '\0';
            ldr[2][l--] = '\0';
        }
        break;

    case '2':
        if(!two) {
            two = true;
            ldr[2][l--] = '\0';
        }
        break;

    case '3':
        three = true;
        break;

    default:
        goto Usage;

    }ARGEND

    if(argc < 2) {
    /*s: [[main]] label [[usage]] (comm.c) */
    Usage:
        fprint(STDERR, "usage: comm [-123] file1 file2\n");
        exits("usage");
    /*e: [[main]] label [[usage]] (comm.c) */
    }

    ib1 = openfil(argv[0]);
    ib2 = openfil(argv[1]);

    if(rd(ib1,lb1) < 0){
        if(rd(ib2,lb2) < 0)
            exits(nil);
        copy(ib2,lb2,2);
    }
    if(rd(ib2,lb2) < 0)
        copy(ib1,lb1,1);

    for(;;){
        switch(compare(lb1,lb2)) {
        case 0:
            wr(lb1,3);
            if(rd(ib1,lb1) < 0) {
                if(rd(ib2,lb2) < 0)
                    exits(nil);
                copy(ib2,lb2,2);
            }
            if(rd(ib2,lb2) < 0)
                copy(ib1,lb1,1);
            continue;

        case 1:
            wr(lb1,1);
            if(rd(ib1,lb1) < 0)
                copy(ib2,lb2,2);
            continue;

        case 2:
            wr(lb2,2);
            if(rd(ib2,lb2) < 0)
                copy(ib1,lb1,1);
            continue;
        }
    }
}
/*e: function [[main]](comm.c) */

/*s: function [[rd]](comm.c) */
int
rd(Biobuf *file, char *buf)
{
    int i, c;

    i = 0;
    while((c = Bgetc(file)) != Beof) {
        *buf = c;
        if(c == '\n' || i > LB-2) {
            *buf = '\0';
            return 0;
        }
        i++;
        buf++;
    }
    return -1;
}
/*e: function [[rd]](comm.c) */
/*s: function [[wr]](comm.c) */
void
wr(char *str, int n)
{

    switch(n){
        case 1:
            if(one)
                return;
            break;

        case 2:
            if(two)
                return;
            break;

        case 3:
            if(three)
                return;
    }
    print("%s%s\n", ldr[n-1],str);
}
/*e: function [[wr]](comm.c) */
/*s: function [[copy]](comm.c) */
void
copy(Biobuf *ibuf, char *lbuf, int n)
{
    do
        wr(lbuf,n);
    while(rd(ibuf,lbuf) >= 0);
    exits(nil);
}
/*e: function [[copy]](comm.c) */
/*s: function [[compare]](comm.c) */
int
compare(char *a, char *b)
{
    while(*a == *b){
        if(*a == '\0')
            return 0;
        a++;
        b++;
    }
    if(*a < *b)
        return 1;
    return 2;
}
/*e: function [[compare]](comm.c) */
/*s: function [[openfil]](comm.c) */
Biobuf*
openfil(char *s)
{
    Biobuf *b;

    if(s[0]=='-' && s[1]=='\0')
        s = "/fd/0";
    b = Bopen(s, OREAD);
    if(b)
        return b;
    fprint(STDERR, "comm: cannot open %s: %r\n",s);
    exits("open");
    // never reached
    return nil;   /* shut up ken */
}
/*e: function [[openfil]](comm.c) */
/*e: compare/comm.c */
