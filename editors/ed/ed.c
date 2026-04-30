/*s: ed/ed.c */
/*
 * Editor
 */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>
#include <regexp.h>

enum
{
    /*s: constants ed.c */
    FNSIZE  = 128,      /* file name */
    /*x: constants ed.c */
    LBSIZE  = 4096,     /* max line size */
    /*x: constants ed.c */
    ESIZE   = 256,      /* max size of reg exp */
    /*x: constants ed.c */
    MAXSUB  = 9,        /* max number of sub reg exp */
    /*x: constants ed.c */
    ESCFLG  = Runemax,  /* escape Rune - user defined code */
    /*x: constants ed.c */
    GBSIZE  = 256,      /* max size of global command */
    /*x: constants ed.c */
    BLKSIZE = 4096,     /* block size in temp file */
    /*x: constants ed.c */
    NBLK    = 8191,     /* max size of temp file */
    /*x: constants ed.c */
    EOF = -1,
    /*e: constants ed.c */
};

// ??? seems dead
void    (*oldhup)(int);
void    (*oldquit)(int);

/*s: globals ed.c */
char*   tfname; // temporary filename (/tmp/eXXXX)
/*x: globals ed.c */
fdt tfile   = -1;
/*x: globals ed.c */
// current write file offset in tfile; 
int tline;
/*x: globals ed.c */
// for w, r, f
char    savedfile[FNSIZE];
/*x: globals ed.c */
ulong   nlall = 128;
// growing_array<int>, initial size = (nlall+ 2+margin)*sizeof(int)
// where the ints are file offsets in tfname corresponding to different lines
int*    zero;
/*x: globals ed.c */
// ref<int> in zero array, current line pointer
int*    dot;
// ref<int> in zero array, last line pointer
int*    dol;
/*x: globals ed.c */
char    line[70];
// ref<char> in line (mark end of line usually)
char*   linp    = line;
/*x: globals ed.c */
Rune    linebuf[LBSIZE];
/*x: globals ed.c */
long count;
/*x: globals ed.c */
int col;
/*x: globals ed.c */
// console buffered input
Biobuf  bcons;
/*x: globals ed.c */
// verbose (a.k.a. interactive) mode
bool vflag   = true;
/*x: globals ed.c */
// output to standard output (instead of editing a file). Useful
// when ed is used as a filter
bool oflag;
/*x: globals ed.c */
// in Linux pid can be very long, so better to have at least 6 X (was 5 before)
// the mkstemp man page recommends 6 X
char template[] = "/tmp/eXXXXXX";
/*x: globals ed.c */
char    T[] = "TMP";
/*x: globals ed.c */
bool fchange;
/*x: globals ed.c */
char    Q[] = "";
/*x: globals ed.c */
//option<char> (0 = None)
int lastc;
//option<char> (0 = None)
int peekc;
/*x: globals ed.c */
// optional programmed command (e.g., "r", "a")
//option<Rune> or list<Rune> when used from global() ?
Rune*   globp;
/*x: globals ed.c */
// ref<int>
int*    addr1;
// ref<int>
int*    addr2;
/*x: globals ed.c */
char    file[FNSIZE];
/*x: globals ed.c */
fdt io;
Biobuf  iobuf;
/*x: globals ed.c */
// write append
bool wrapp;
/*x: globals ed.c */
bool given;
/*x: globals ed.c */
Reprog  *pattern;
/*x: globals ed.c */
Resub   subexp[MAXSUB];
/*x: globals ed.c */
Rune*   loc1;
Rune*   loc2;
/*x: globals ed.c */
Rune    rhsbuf[LBSIZE/sizeof(Rune)];
/*x: globals ed.c */
int subnewa;
/*x: globals ed.c */
Rune    genbuf[LBSIZE];
/*x: globals ed.c */
int subolda;
/*x: globals ed.c */
int names[26];
/*x: globals ed.c */
int anymarks;
/*x: globals ed.c */
bool waiting;
/*x: globals ed.c */
// for displaying special chars, 'l' list flag
bool listf;
/*x: globals ed.c */
char    hex[]   = "0123456789abcdef";
/*x: globals ed.c */
bool pflag;
/*x: globals ed.c */
// 'n' flag
bool listn;
/*x: globals ed.c */
int bpagesize = 20;
/*x: globals ed.c */
int nleft;
/*x: globals ed.c */
Rune*   linebp;
/*x: globals ed.c */
int iblock;
int oblock;
int ichanged;
/*x: globals ed.c */
jmp_buf savej;
/*x: globals ed.c */
bool rescuing;
/*e: globals ed.c */

// forward declarations
void    add(int);
int*    address(void);
int     append(int(*)(void), int*);
void    browse(void);
void    callunix(void);
void    commands(void);
void    compile(int);
int     compsub(void);
void    dosub(void);
void    error(char*);
int     match(int*);
void    exfile(int);
void    filename(int);
Rune*   getblock(int, int);
int     getchr(void);
int     getcopy(void);
int     getfile(void);
Rune*   getline(int);
int     getnum(void);
int     getsub(void);
int     gettty(void);
void    global(int);
void    init(void);
void    join(void);
void    move(int);
void    newline(void);
void    nonzero(void);
void    notifyf(void*, char*);
Rune*   place(Rune*, Rune*, Rune*);
void    printcom(void);
void    putchr(int);
void    putd(void);
void    putfile(void);
int     putline(void);
void    putshst(Rune*);
void    putst(char*);
void    quit(void);
void    rdelete(int*, int*);
void    regerror(char *);
void    reverse(int*, int*);
void    setnoaddr(void);
void    setwide(void);
void    squeeze(int);
void    substitute(int);

/*s: function [[main]](ed.c) */
void
main(int argc, char *argv[])
{
    char *p1, *p2;

    Binit(&bcons, STDIN, OREAD);
    notify(notifyf);

    ARGBEGIN {
    /*s: [[main()]](ed.c) flags processing cases */
    case 'o':
        oflag = true;
        vflag = false;
        break;
    /*e: [[main()]](ed.c) flags processing cases */
    } ARGEND

    USED(argc);
    /*s: [[main()]](ed.c) if [[-]] flag */
    if(*argv && (strcmp(*argv, "-") == ORD__EQ)) {
        argv++;
        vflag = false;
    }
    /*e: [[main()]](ed.c) if [[-]] flag */
    /*s: [[main()]](ed.c) if [[oflag]] */
    if(oflag) {
        p1 = "/fd/1";
        p2 = savedfile;
        while(*p2++ = *p1++)
            ;
        globp = L"a";
    }
    /*e: [[main()]](ed.c) if [[oflag]] */
    else if(*argv) {
        p1 = *argv;
        p2 = savedfile;
        while(*p2++ = *p1++)
            if(p2 >= &savedfile[sizeof(savedfile)])
                p2--;
        globp = L"r";
    }
    zero = malloc((nlall+5)*sizeof(int*)); // BUG should be sizeof(int)
    tfname = mktemp(template);

    init();
    /*s: [[main()]](ed.c) before [[commands()]] */
    setjmp(savej);
    /*e: [[main()]](ed.c) before [[commands()]] */
    commands();
    quit();
}
/*e: function [[main]](ed.c) */

/*s: function [[commands]](ed.c) */
void
commands(void)
{
    int c;
    /*s: [[commands()]] other locals */
    int temp;
    /*x: [[commands()]] other locals */
    int *a1;
    /*x: [[commands()]] other locals */
    char lastsep; // '\n' or ',' or ';'
    /*x: [[commands()]] other locals */
    Dir *d;
    /*e: [[commands()]] other locals */

    for(;;) {
        /*s: [[commands()]] in for loop, if [[pflag]] */
        if(pflag) {
            pflag = false;
            addr1 = addr2 = dot;
            printcom();
        }
        /*e: [[commands()]] in for loop, if [[pflag]] */
        /*s: [[commands()]] read [[addr1]] and [[c]] via [[getchr]] */
        c = '\n';
        addr1 = nil;

        for(;;) {
            lastsep = c;
            a1 = address();
            c = getchr();
 
           if(c != ',' && c != ';')
                break;

            // else
            if(lastsep == ',')
                error(Q);
            if(a1 == nil) {
                a1 = zero+1; // line 1
                if(a1 > dol)
                    a1--;
            }
            addr1 = a1;
            /*s: [[commands()]] in address parsing, if separator is [[';']] */
            if(c == ';')
                dot = a1;
            /*e: [[commands()]] in address parsing, if separator is [[';']] */
        }
        /*s: [[commands()]] after address parsing, use defaults if missing addresses */
        if(lastsep != '\n' && a1 == nil)
            a1 = dol;

        if((addr2=a1) == nil) {
            given = false;
            addr2 = dot;    
        } else
            given = true;

        if(addr1 == nil)
            addr1 = addr2;
        /*e: [[commands()]] after address parsing, use defaults if missing addresses */
        /*e: [[commands()]] read [[addr1]] and [[c]] via [[getchr]] */
        switch(c) {
        /*s: [[commands()]] switch [[c]] cases (ed.c) */
        case EOF:
            return;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'r':
            // will set file[]
            filename(c);
        caseread:
            if((io=open(file, OREAD)) < 0) {
                lastc = '\n';
                error(file);
            }
            /*s: [[commands()]] in [[r]] case if append only file */
            if((d = dirfstat(io)) != nil){
                if(d->mode & DMAPPEND)
                    print("warning: %s is append only\n", file);
                free(d);
            }
            /*e: [[commands()]] in [[r]] case if append only file */
            Binit(&iobuf, io, OREAD);
            setwide();
            squeeze(0);
            c = (zero != dol);
            // getfile() will use iobuf
            append(getfile, addr2);
            exfile(OREAD); // will close io

            fchange = c;
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'W':
            wrapp = true;
            // Fallthrough
        case 'w':
            setwide();
            squeeze(dol>zero);

            /*s: [[commands()]] when [['w']] check for [['q']] part1 */
            temp = getchr();
            if(temp != 'q' && temp != 'Q') {
                peekc = temp;
                temp = 0;
            }
            /*e: [[commands()]] when [['w']] check for [['q']] part1 */
            filename(c);

            if(!wrapp ||
              ((io = open(file, OWRITE)) == -1) ||
              ((seek(io, 0L, SEEK__END)) == -1))

                if((io = create(file, OWRITE, 0666)) < 0)
                    error(file);

            Binit(&iobuf, io, OWRITE);
            wrapp = false;
            if(dol > zero)
                putfile();

            exfile(OWRITE);

            if(addr1<=zero+1 && addr2==dol)
                fchange = false;
            /*s: [[commands()]] when [['w']] check for [['q']] part2 */
            if(temp == 'Q')
                fchange = false;
            if(temp)
                quit();
            /*e: [[commands()]] when [['w']] check for [['q']] part2 */
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        /*s: [[commands]] before [['p']] case */
        case 'l':
            listf = true;
            // fallthrough:
        /*e: [[commands]] before [['p']] case */
        case 'p':
        case 'P':
            newline();
            printcom();
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case '\n':
            if(a1==nil) {
                a1 = dot+1;
                addr2 = a1;
                addr1 = a1;
            }
            if(lastsep==';')
                addr1 = a1;

            printcom();
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'f':
            setnoaddr();
            filename(c);
            putst(savedfile);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case '=':
            setwide();
            squeeze(0);
            newline();
            count = addr2 - zero;
            putd();
            putchr(L'\n');
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'a':
            add(0);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'i':
            add(-1);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'Q':
            fchange = false;
            // fallthrough:
        case 'q':
            setnoaddr();
            newline();
            quit();
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'd':
            nonzero();
            newline();
            rdelete(addr1, addr2);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'c':
            nonzero();
            newline();
            rdelete(addr1, addr2);
            append(gettty, addr1-1);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'm':
            move(0);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 't':
            move(1);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 's':
            nonzero();
            substitute(globp != nil);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'u':
            nonzero();
            newline();
            if((*addr2&~01) != subnewa)
                error(Q);
            *addr2 = subolda;
            dot = addr2;
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'k':
            nonzero();
            c = getchr();
            if(c < 'a' || c > 'z')
                error(Q);
            newline();
            names[c-'a'] = *addr2 & ~01;
            anymarks |= 01;
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'g':
            global(1);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'v':
            global(0);
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case '!':
            callunix();
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'n':
            listn = true;
            newline();
            printcom();
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'j':
            if(!given)
                addr2++;
            newline();
            join();
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'b':
            nonzero();
            browse();
            continue;
        /*x: [[commands()]] switch [[c]] cases (ed.c) */
        case 'E':
            fchange = false;
            c = 'e';
            // Fallthrough
        case 'e':
            setnoaddr();
            if(vflag && fchange) {
                fchange = false;
                error(Q);
            }
            filename(c);
            init();
            addr2 = zero;
            goto caseread;
        /*e: [[commands()]] switch [[c]] cases (ed.c) */
        }
        error(Q);
    }
}
/*e: function [[commands]](ed.c) */

// Command helpers
/*s: function [[printcom]](ed.c) */
void
printcom(void)
{
    int *a1;

    nonzero();
    a1 = addr1;
    do {
        /*s: [[printcom()]] if [[listn]] */
        if(listn) {
            count = a1-zero;
            putd();
            putchr(L'\t');
        }
        /*e: [[printcom()]] if [[listn]] */
        putshst(getline(*a1++));
    } while(a1 <= addr2);
    dot = addr2;
    /*s: [[printcom()]] reset flags */
    listf = false;
    listn = false;
    pflag = false;
    /*e: [[printcom()]] reset flags */
}
/*e: function [[printcom]](ed.c) */

/*s: function [[address]](ed.c) */
int*
address(void)
{
    int sign, *a, opcnt, nextopand, *b, c;

    nextopand = -1;
    sign = 1;
    opcnt = 0;
    a = dot;
    do {
        do {
            c = getchr();
        } while(c == ' ' || c == '\t');

        if(c >= '0' && c <= '9') {
            peekc = c;
            if(!opcnt)
                a = zero;
            a += sign*getnum();
        } else

        switch(c) {
        // will return in default: case
        /*s: [[address()]](ed.c) switch [[c]] cases */
        default:
            if(nextopand == opcnt) {
                a += sign;
                if(a < zero || dol < a)
                    continue;       /* error(Q); */
            }

            if(c != '+' && c != '-' && c != '^') {
                peekc = c;
                if(opcnt == 0)
                    a = nil;

                // finally returning!
                return a;
            }
            sign = 1;
            if(c != '+')
                sign = -sign;
            nextopand = ++opcnt;
            continue;
        /*x: [[address()]](ed.c) switch [[c]] cases */
        case '$':
            a = dol;
            // Fallthrough
        case '.':
            if(opcnt)
                error(Q);
            break;
        /*x: [[address()]](ed.c) switch [[c]] cases */
        case '?':
            sign = -sign;
            // Fallthrough
        case '/':
            compile(c);
            b = a;
            for(;;) {
                // direction
                a += sign;
                // wrap around
                if(a <= zero)
                    a = dol;
                if(a > dol)
                    a = zero;

                if(match(a))
                    break;
                // reached back where we were without finding anything
                if(a == b)
                    error(Q);
            }
            break;
        /*x: [[address()]](ed.c) switch [[c]] cases */
        case '\'':
            c = getchr();
            if(opcnt || c < 'a' || c > 'z')
                error(Q);
            a = zero;
            do {
                a++;
            } while(a <= dol && names[c-'a'] != (*a & ~01));
            break;

        /*e: [[address()]](ed.c) switch [[c]] cases */
        }

        sign = 1;
        opcnt++;
    } while(zero <= a && a <= dol);

    error(Q);
    return nil;
}
/*e: function [[address]](ed.c) */
/*s: function [[getnum]](ed.c) */
int
getnum(void)
{
    int r = 0;
    int c;

    for(;;) {
        c = getchr();
        if(c < '0' || c > '9')
            break;
        r = r*10 + (c-'0');
    }
    peekc = c;
    return r;
}
/*e: function [[getnum]](ed.c) */

// ???
/*s: function [[setwide]](ed.c) */
/// main -> commands('r') -> <>
void
setwide(void)
{
    if(!given) {
        addr1 = zero + (dol>zero);
        addr2 = dol;
    }
}
/*e: function [[setwide]](ed.c) */
/*s: function [[setnoaddr]](ed.c) */
void
setnoaddr(void)
{
    if(given)
        error(Q);
}
/*e: function [[setnoaddr]](ed.c) */
/*s: function [[nonzero]](ed.c) */
void
nonzero(void)
{
    squeeze(1);
}
/*e: function [[nonzero]](ed.c) */
/*s: function [[squeeze]](ed.c) */
/// main -> commands('r') -> <>
void
squeeze(int i)
{
    if(addr1 < zero+i || addr2 > dol || addr1 > addr2)
        error(Q);
}
/*e: function [[squeeze]](ed.c) */
/*s: function [[newline]](ed.c) */
void
newline(void)
{
    int c;

    c = getchr();
    if(c == '\n' || c == EOF)
        return;
    /*s: [[newline()]] if special chars [[pln]] */
    if(c == 'p' || c == 'l' || c == 'n') {
        pflag = true;
        if(c == 'l')
            listf = true;
        else
        if(c == 'n')
            listn = true;
        c = getchr();
        if(c == '\n')
            return;
    }
    /*e: [[newline()]] if special chars [[pln]] */
    // else
    error(Q);
}
/*e: function [[newline]](ed.c) */
/*s: function [[filename]](ed.c) */
/// main -> commands('r') -> <>
void
filename(int comm)
{
    char *p1, *p2;
    Rune rune;
    int c;

    count = 0;
    c = getchr();
    /*s: [[filename()]] if [[c]] is newline or EOF */
    if(c == '\n' || c == EOF) {
        p1 = savedfile;
        if(*p1 == '\0' && comm != 'f')
            error(Q);
        p2 = file;
        while(*p2++ = *p1++)
            ;
        return;
    }
    /*e: [[filename()]] if [[c]] is newline or EOF */
    // else
    /*s: [[filename()]] read a space and skip extra spaces */
    if(c != ' ')
        error(Q);
    while((c=getchr()) == ' ')
        ;
    if(c == '\n')
        error(Q);
    /*e: [[filename()]] read a space and skip extra spaces */
    p1 = file;
    do {
        if(p1 >= &file[sizeof(file)-6] || c == ' ' || c == EOF)
            error(Q);
        rune = c;
        p1 += runetochar(p1, &rune);
    } while((c=getchr()) != '\n');
    *p1 = '\0';
    
    /*s: [[filename()]] set [[savedfile]] depending on commands */
    if(savedfile[0] == '\0' || comm == 'e' || comm == 'f') {
        p1 = savedfile;
        p2 = file;
        while(*p1++ = *p2++)
            ;
    }
    /*e: [[filename()]] set [[savedfile]] depending on commands */
}
/*e: function [[filename]](ed.c) */

// Writing files
/*s: function [[exfile]](ed.c) */
/// main -> commands('r') -> <>
void
exfile(int om)
{

    if(om == OWRITE)
        if(Bflush(&iobuf) < 0)
            error(Q);
    close(io);
    io = -1;
    if(vflag) {
        putd();
        putchr(L'\n');
    }
}
/*e: function [[exfile]](ed.c) */

// Error management
/*s: function [[error_1]](ed.c) */
void
error_1(char *s)
{
    int c;

    /*s: [[error_1()]](ed.c)) reset globals */
    wrapp = false;
    listf = false;
    listn = false;
    count = 0;

    seek(STDIN, 0, SEEK__END);
    pflag = false;

    if(globp)
        lastc = '\n';
    globp = nil;

    peekc = lastc;
    if(lastc)
        for(;;) {
            c = getchr();
            if(c == '\n' || c == EOF)
                break;
        }

    if(io > 0) {
        close(io);
        io = -1;
    }
    /*e: [[error_1()]](ed.c)) reset globals */
    putchr(L'?');
    putst(s);
}
/*e: function [[error_1]](ed.c) */
/*s: function [[error]](ed.c) */
void
error(char *s)
{
    error_1(s);
    longjmp(savej, 1);
}
/*e: function [[error]](ed.c) */
/*s: function [[rescue]](ed.c) */
void
rescue(void)
{
    rescuing = true;
    if(dol > zero) {
        addr1 = zero+1;
        addr2 = dol;
        io = create("ed.hup", OWRITE, 0666);
        if(io > 0){
            Binit(&iobuf, io, OWRITE);
            putfile();
        }
    }
    fchange = false;
    quit();
}
/*e: function [[rescue]](ed.c) */

// Note management
/*s: function [[notifyf]](ed.c) */
void
notifyf(void *a, char *s)
{
    if(strcmp(s, "interrupt") == ORD__EQ){
        if(rescuing || waiting)
            noted(NCONT);
        putchr(L'\n');
        lastc = '\n';
        error_1(Q);
        notejmp(a, savej, 0);
    }
    if(strcmp(s, "hangup") == ORD__EQ){
        if(rescuing)
            noted(NDFLT);
        rescue();
    }
    fprint(STDERR, "ed: note: %s\n", s);
    abort();
}
/*e: function [[notifyf]](ed.c) */

// Reading characters
/*s: function [[getchr]](ed.c) */
int
getchr(void)
{
    if(lastc = peekc) {
        peekc = 0;
        return lastc;
    }
    // else
    if(globp) {
        if((lastc=*globp++) != 0)
            return lastc;
        // else
        globp = nil;
        return EOF;
    }
    // else
    lastc = Bgetrune(&bcons);
    return lastc;
}
/*e: function [[getchr]](ed.c) */
/*s: function [[gety]](ed.c) */
/// gettty -> <>
int
gety(void)
{
    int c;
    Rune *p = linebuf;
    /*s: [[gety()]] other locals */
    Rune *gf = globp;
    /*e: [[gety()]] other locals */
    for(;;) {
        c = getchr();
        if(c == '\n') {
            *p = 0;
            return 0;
        }
        // else
        if(c == EOF) {
            /*s: [[<<get()]] if [[gf]] */
            if(gf)
                peekc = c;
            /*e: [[<<get()]] if [[gf]] */
            return c;
        }
        // else
        if(c == 0)
            continue;
        // else
        *p++ = c;
        if(p >= &linebuf[LBSIZE-sizeof(Rune)])
            error(Q);
    }
}
/*e: function [[gety]](ed.c) */
/*s: function [[gettty]](ed.c) */
/// main -> commands('a') -> add -> <>
int
gettty(void)
{
    int rc;

    rc = gety();
    if(rc)
        return rc;
    // else
    if(linebuf[0] == '.' && linebuf[1] == '\0')
        return EOF;
    return 0; // OK_0 ?
}
/*e: function [[gettty]](ed.c) */

// Reading and writing files
/*s: function [[getfile]](ed.c) */
/// main -> commands(c = 'r') -> append(<>, ...) -> <>
int
getfile(void)
{
    int c;
    Rune *lp;

    lp = linebuf;
    do {
        c = Bgetrune(&iobuf);
        /*s: [[getfile()]] if [[c]] negative, possibly return [[EOF]] */
        if(c < 0) {
            if(lp > linebuf) {
                putst("'\\n' appended");
                c = '\n';
            } else
                return EOF;
        }
        /*e: [[getfile()]] if [[c]] negative, possibly return [[EOF]] */
        // else
        /*s: [[getfile()]] if overflow [[linebuf]] */
        if(lp >= &linebuf[LBSIZE]) {
            lastc = '\n';
            error(Q);
        }
        /*e: [[getfile()]] if overflow [[linebuf]] */
        // else
        *lp++ = c;
        count++;
    } while(c != '\n');
    lp[-1] = 0;
    return 0; // OK_0
}
/*e: function [[getfile]](ed.c) */
/*s: function [[putfile]](ed.c) */
void
putfile(void)
{
    int *a1;
    Rune *lp;
    long c;

    a1 = addr1;
    do {
        lp = getline(*a1++);
        for(;;) {
            count++;
            c = *lp++;
            if(c == 0) {
                if(Bputrune(&iobuf, '\n') < 0)
                    error(Q);
                break;
            }
            if(Bputrune(&iobuf, c) < 0)
                error(Q);
        }
    } while(a1 <= addr2);
    if(Bflush(&iobuf) < 0)
        error(Q);
}
/*e: function [[putfile]](ed.c) */

/*s: function [[append]](ed.c) */
/// main -> commands('r') -> <>
int
append(int (*f)(void), int *a)
{
    //ref<int> in zero[]
    int *a1, *a2, *rdot;
    int nline = 0;
    // file offset in tfile for temporary line just added by putline()
    int tl;

    dot = a;
    // f() (e.g., getfile()) will modify linebuf
    while((*f)() == 0) {
        /*s: [[append()]] grow [[zero]] if [[zero]] too small */
        if((dol-zero) >= nlall) {

            nlall += 512;
            a1 = realloc(zero, (nlall+5)*sizeof(int*)); // BUG: sizeof(int)
            if(a1 == nil) {
                error("MEM?");
                rescue();
            }
            tl = a1 - zero; /* relocate pointers */

            zero += tl;
            addr1 += tl;
            addr2 += tl;
            dol += tl;
            dot += tl;
        }
        /*e: [[append()]] grow [[zero]] if [[zero]] too small */
        // putline() will use linebuf
        tl = putline();
        nline++;

        // set zero[] indices
        a1 = ++dol;
        a2 = a1+1;
        rdot = ++dot;
        // shift existing line references up by one
        while(a1 > rdot)
            *--a2 = *--a1;
        // insert the new line reference in zero
        *rdot = tl;
    }
    return nline;
}
/*e: function [[append]](ed.c) */
/*s: function [[add]](ed.c) */
void
add(int i)
{
    if(i && (given || dol > zero)) {
        addr1--;
        addr2--;
    }
    squeeze(0);
    newline();
    append(gettty, addr2);
}
/*e: function [[add]](ed.c) */

/*s: function [[browse]](ed.c) */
void
browse(void)
{
    int forward, n;
    static int bformat, bnum; /* 0 */

    forward = 1;
    peekc = getchr();
    if(peekc != '\n'){
        if(peekc == '-' || peekc == '+') {
            if(peekc == '-')
                forward = 0;
            getchr();
        }
        n = getnum();
        if(n > 0)
            bpagesize = n;
    }
    newline();
    if(pflag) {
        bformat = listf;
        bnum = listn;
    } else {
        listf = bformat;
        listn = bnum;
    }
    if(forward) {
        addr1 = addr2;
        addr2 += bpagesize;
        if(addr2 > dol)
            addr2 = dol;
    } else {
        addr1 = addr2-bpagesize;
        if(addr1 <= zero)
            addr1 = zero+1;
    }
    printcom();
}
/*e: function [[browse]](ed.c) */

/*s: function [[callunix]](ed.c) */
void
callunix(void)
{
    int c, pid;
    Rune rune;
    char buf[512];
    char *p;

    setnoaddr();

    p = buf;
    while((c=getchr()) != EOF && c != '\n')
        if(p < &buf[sizeof(buf) - 6]) {
            rune = c;
            p += runetochar(p, &rune);
        }
    *p = '\0';

    pid = fork();
    if(pid == 0) {
        // child
        execl("/bin/rc", "rc", "-c", buf, nil);
        // should not be reached
        exits("execl failed");
    }
    // else, parent
    waiting = true;
    while(waitpid() != pid)
        ;
    waiting = false;
    if(vflag)
        putst("!");
}
/*e: function [[callunix]](ed.c) */

/*s: function [[quit]](ed.c) */
/// main | commands('q') | ... -> <>
void
quit(void)
{
    if(vflag && fchange && dol!=zero) {
        fchange = false;
        error(Q);
    }
    remove(tfname);
    exits(nil);
}
/*e: function [[quit]](ed.c) */
/*s: function [[onquit]](ed.c) */
void
onquit(int sig)
{
    USED(sig);
    quit();
}
/*e: function [[onquit]](ed.c) */

// Delete
/*s: function [[rdelete]](ed.c) */
void
rdelete(int *ad1, int *ad2)
{
    int *a1, *a2, *a3;

    a1 = ad1;
    a2 = ad2+1;
    a3 = dol;
    dol -= a2 - a1;
    do {
        *a1++ = *a2++;
    } while(a2 <= a3);
    a1 = ad1;
    if(a1 > dol)
        a1 = dol;
    dot = a1;
    fchange = true;
}
/*e: function [[rdelete]](ed.c) */
/*s: function [[gdelete]](ed.c) */
void
gdelete(void)
{
    int *a1, *a2, *a3;

    a3 = dol;
    for(a1=zero; (*a1&01)==0; a1++)
        if(a1>=a3)
            return;
    for(a2=a1+1; a2<=a3;) {
        if(*a2 & 01) {
            a2++;
            dot = a1;
        } else
            *a1++ = *a2++;
    }
    dol = a1-1;
    if(dot > dol)
        dot = dol;
    fchange = true;
}
/*e: function [[gdelete]](ed.c) */

// Get/Put lines
/*s: function [[getline]](ed.c) */
/// printcom | putfile -> <>
Rune*
getline(int tl)
{
    Rune *lp, *bp;
    int nl;

    lp = linebuf;
    bp = getblock(tl, OREAD);
    nl = nleft;
    tl &= ~((BLKSIZE/sizeof(Rune)) - 1);
    while(*lp++ = *bp++) {
        nl -= sizeof(Rune);
        if(nl == 0) {
            tl += BLKSIZE/sizeof(Rune);
            bp = getblock(tl, OREAD);
            nl = nleft;
        }
    }
    return linebuf;
}
/*e: function [[getline]](ed.c) */
/*s: function [[putline]](ed.c) */
/// main -> commands('r') -> append -> <>
int
putline(void)
{
    Rune *lp, *bp;
    int nl, tl;

    fchange = true;
    lp = linebuf;
    tl = tline;

    bp = getblock(tl, OWRITE);
    nl = nleft;
    tl &= ~((BLKSIZE/sizeof(Rune))-1);
    while(*bp = *lp++) {
        if(*bp++ == '\n') {
            bp[-1] = 0;
            linebp = lp;
            break;
        }
        nl -= sizeof(Rune);
        if(nl == 0) {
            tl += BLKSIZE/sizeof(Rune);
            bp = getblock(tl, OWRITE);
            nl = nleft;
        }
    }

    nl = tline;
    tline += ((lp-linebuf) + 03) & 077776;
    return nl;
}
/*e: function [[putline]](ed.c) */
/*s: function [[blkio]](ed.c) */
void
blkio(int b, uchar *buf, long (*iofcn)(int, void *, long))
{
    seek(tfile, b*BLKSIZE, SEEK__START);
    if((*iofcn)(tfile, buf, BLKSIZE) != BLKSIZE) {
        error(T);
    }
}
/*e: function [[blkio]](ed.c) */
/*s: function [[getblock]](ed.c) */
/// putline | getline -> <>
Rune*
getblock(int atl, int iof)
{
    int bno; // block number
    int off; // offset
    
    static uchar ibuff[BLKSIZE];
    static uchar obuff[BLKSIZE];

    bno = atl / (BLKSIZE/sizeof(Rune));
    /* &~3 so the ptr is aligned to 4 (?) */
    off = (atl*sizeof(Rune)) & (BLKSIZE-1) & ~3;
    if(bno >= NBLK) {
        lastc = '\n';
        error(T);
    }
    nleft = BLKSIZE - off;
    if(bno == iblock) {
        ichanged |= iof;
        return (Rune*)(ibuff+off);
    }
    if(bno == oblock)
        return (Rune*)(obuff+off);
    if(iof == OREAD) {
        if(ichanged)
            blkio(iblock, ibuff, write);
        ichanged = 0;
        iblock = bno;
        blkio(bno, ibuff, read);
        return (Rune*)(ibuff+off);
    }
    if(oblock >= 0)
        blkio(oblock, obuff, write);
    oblock = bno;
    return (Rune*)(obuff+off);
}
/*e: function [[getblock]](ed.c) */

/*s: function [[init]](ed.c) */
/// main | commands('e') -> <>
void
init(void)
{
    /*s: [[init()]](ed.c) locals */
    int *markp;
    /*e: [[init()]](ed.c) locals */

    close(tfile);
    /*s: [[init()]](ed.c) initializing globals */
    tline = 2;
    /*x: [[init()]](ed.c) initializing globals */
    subnewa = 0;
    /*x: [[init()]](ed.c) initializing globals */
    anymarks = 0;
    /*x: [[init()]](ed.c) initializing globals */
    for(markp = names; markp < &names[26]; )
        *markp++ = 0;
    /*x: [[init()]](ed.c) initializing globals */
    iblock = -1;
    oblock = -1;
    ichanged = 0;
    /*e: [[init()]](ed.c) initializing globals */
    if((tfile = create(tfname, ORDWR, 0600)) < 0){
        error_1(T);
        exits(nil);
    }
    dot = dol = zero;
}
/*e: function [[init]](ed.c) */

/*s: function [[global]](ed.c) */
void
global(int k)
{
    Rune *gp, globuf[GBSIZE];
    int c, *a1;

    if(globp)
        error(Q);

    setwide();
    squeeze(dol > zero);

    // readding a '/' or '?'
    c = getchr();
    if(c == '\n')
        error(Q);

    // reading the whole pattern until corresponding ending '/' or '?'
    compile(c);

    gp = globuf;
    while((c=getchr()) != '\n') {
        if(c == EOF)
            error(Q);
        if(c == '\\') {
            c = getchr();
            if(c != '\n')
                *gp++ = '\\';
        }
        *gp++ = c;
        if(gp >= &globuf[GBSIZE-2])
            error(Q);
    }
    if(gp == globuf)
        *gp++ = 'p';
    *gp++ = '\n';
    *gp = 0;

    for(a1=zero; a1<=dol; a1++) {
        *a1 &= ~01;
        if(a1 >= addr1 && a1 <= addr2 && match(a1) == k)
            *a1 |= 01;
    }

    /*s: [[global()]](ed.c) if [[g/.../d]] command, call optimized [[gdelete()]] */
    /*
     * Special case: g/.../d (avoid n^2 algorithm)
     */
    if(globuf[0] == 'd' && globuf[1] == '\n' && globuf[2] == 0) {
        gdelete();
        return;
    }
    /*e: [[global()]](ed.c) if [[g/.../d]] command, call optimized [[gdelete()]] */

    for(a1=zero; a1<=dol; a1++) {
        if(*a1 & 01) {
            *a1 &= ~01;
            dot = a1;
            globp = globuf;
            // recurse!
            commands();
            a1 = zero; // zero may have grown and move, need update a1
        }
    }
}
/*e: function [[global]](ed.c) */

/*s: function [[join]](ed.c) */
void
join(void)
{
    Rune *gp, *lp;
    int *a1;

    nonzero();
    gp = genbuf;
    for(a1=addr1; a1<=addr2; a1++) {
        lp = getline(*a1);
        while(*gp = *lp++)
            if(gp++ >= &genbuf[LBSIZE-sizeof(Rune)])
                error(Q);
    }
    lp = linebuf;
    gp = genbuf;
    while(*lp++ = *gp++)
        ;
    *addr1 = putline();
    if(addr1 < addr2)
        rdelete(addr1+1, addr2);
    dot = addr1;
}
/*e: function [[join]](ed.c) */

// Search and replace
/*s: function [[substitute]](ed.c) */
void
substitute(int inglob)
{
    int *a1;
    bool gsubf; // s/.../.../g  global subst
    int n = 0;
    /*s: [[substitute()]](ed.c) other locals */
    int *mp, nl;
    /*e: [[substitute()]](ed.c) other locals */

    /*s: [[substitute()]](ed.c) read optional [[n]] */
    n = getnum();   /* OK even if n==0 */
    /*e: [[substitute()]](ed.c) read optional [[n]] */

    gsubf = compsub();

    for(a1 = addr1; a1 <= addr2; a1++) {
        // will internally set linebuf[]
        if(match(a1)){

            int *ozero;
            int m = n;

            do {
                int span = loc2-loc1;

                if(--m <= 0) {
                    // will modify linebuf[]
                    dosub();

                    if(!gsubf)
                        break;
                    // else
                    if(span == 0) { /* null RE match */
                        if(*loc2 == 0)
                            break;
                        loc2++;
                    }
                }
            } while(match(nil));

            if(m <= 0) {
                inglob |= 01;
                // will use linebuf[]
                subnewa = putline();
                *a1 &= ~01;
                /*s: [[substitute()]](ed.c) after [[putline()]] if [[anymarks]] */
                if(anymarks) {
                    for(mp=names; mp<&names[26]; mp++)
                        if(*mp == *a1)
                            *mp = subnewa;
                }
                /*e: [[substitute()]](ed.c) after [[putline()]] if [[anymarks]] */
                /*s: [[substitute()]](ed.c) after [[putline()]] set [[subolda]] */
                subolda = *a1;
                /*e: [[substitute()]](ed.c) after [[putline()]] set [[subolda]] */
                *a1 = subnewa;

                /*s: [[substitute()]](ed.c) after [[putline()]] call [[append()]] */
                ozero = zero;
                nl = append(getsub, a1);
                addr2 += nl;
                nl += zero-ozero;
                a1 += nl;
                /*e: [[substitute()]](ed.c) after [[putline()]] call [[append()]] */
            }
        }
    }
    if(inglob == 0)
        error(Q);
}
/*e: function [[substitute]](ed.c) */
/*s: function [[compsub]](ed.c) */
bool
compsub(void)
{
    int seof, c;
    Rune *p;

    seof = getchr();
    if(seof == '\n' || seof == ' ')
        error(Q);

    // read (and compile) the regexp (left hand side)
    compile(seof);

    // read the subst (right hand side)
    p = rhsbuf;
    for(;;) {
        c = getchr();
        /*s: [[compsub()]](ed.c) if match group */
        if(c == '\\') {
            c = getchr();
            *p++ = ESCFLG;
            /*s: [[compsub()]](ed.c) sanity check [[p]] inside [[rhsbuf]] */
            if(p >= &rhsbuf[LBSIZE/sizeof(Rune)])
                error(Q);
            /*e: [[compsub()]](ed.c) sanity check [[p]] inside [[rhsbuf]] */
        } 
        /*e: [[compsub()]](ed.c) if match group */
        else
        /*s: [[compsub()]](ed.c) if newline and no [[globp]] */
        if(c == '\n' && (!globp || !globp[0])) {
            peekc = c;
            pflag = true;
            break;
        }
        /*e: [[compsub()]](ed.c) if newline and no [[globp]] */
        else
          if(c == seof)
              break;
        // else
        *p++ = c;
        /*s: [[compsub()]](ed.c) sanity check [[p]] inside [[rhsbuf]] */
        if(p >= &rhsbuf[LBSIZE/sizeof(Rune)])
            error(Q);
        /*e: [[compsub()]](ed.c) sanity check [[p]] inside [[rhsbuf]] */
    }
    *p = 0;
    /*s: [[compsub()]](ed.c) peek to check for [['g']] */
    peekc = getchr();
    if(peekc == 'g') {
        peekc = 0;
        newline();
        return true;
    }
    /*e: [[compsub()]](ed.c) peek to check for [['g']] */
    // else
    newline();
    return false;
}
/*e: function [[compsub]](ed.c) */
/*s: function [[getsub]](ed.c) */
int
getsub(void)
{
    Rune *p1, *p2;

    p1 = linebuf;
    if((p2 = linebp) == 0)
        return EOF;
    while(*p1++ = *p2++)
        ;
    linebp = 0;
    return 0;
}
/*e: function [[getsub]](ed.c) */
/*s: function [[dosub]](ed.c) */
void
dosub(void)
{
    Rune *lp, *sp, *rp;
    int c, n;

    lp = linebuf;
    sp = genbuf;
    rp = rhsbuf;
    while(lp < loc1)
        *sp++ = *lp++;
    while(c = *rp++) {
        /*s: [[dosub()]](ed.c) if [[c == '&']] */
        if(c == '&'){
            sp = place(sp, loc1, loc2);
            continue;
        }
        /*e: [[dosub()]](ed.c) if [[c == '&']] */
        /*s: [[dosub()]](ed.c) if match group */
        if(c == ESCFLG && (c = *rp++) >= '1' && c < MAXSUB+'0') {
            n = c-'0';
            if(subexp[n].s.rsp && subexp[n].e.rep) {
                sp = place(sp, subexp[n].s.rsp, subexp[n].e.rep);
                continue;
            }
            error(Q);
        }
        /*e: [[dosub()]](ed.c) if match group */
        // else
        *sp++ = c;
        if(sp >= &genbuf[LBSIZE])
            error(Q);
    }
    lp = loc2;
    loc2 = sp - genbuf + linebuf;
    while(*sp++ = *lp++)
        if(sp >= &genbuf[LBSIZE])
            error(Q);
    lp = linebuf;
    sp = genbuf;
    while(*lp++ = *sp++)
        ;
}
/*e: function [[dosub]](ed.c) */
/*s: function [[place]](ed.c) */
Rune*
place(Rune *sp, Rune *l1, Rune *l2)
{

    while(l1 < l2) {
        *sp++ = *l1++;
        if(sp >= &genbuf[LBSIZE])
            error(Q);
    }
    return sp;
}
/*e: function [[place]](ed.c) */

/*s: function [[move]](ed.c) */
void
move(int cflag)
{
    int *adt, *ad1, *ad2;

    nonzero();
    if((adt = address())==0)    /* address() guarantees addr is in range */
        error(Q);
    newline();

    if(cflag) {
        int *ozero, delta;
        ad1 = dol;
        ozero = zero;
        append(getcopy, ad1++);
        ad2 = dol;
        delta = zero - ozero;
        ad1 += delta;
        adt += delta;
    } else {
        ad2 = addr2;
        for(ad1 = addr1; ad1 <= ad2;)
            *ad1++ &= ~01;
        ad1 = addr1;
    }
    ad2++;
    if(adt<ad1) {
        dot = adt + (ad2-ad1);
        if((++adt)==ad1)
            return;
        reverse(adt, ad1);
        reverse(ad1, ad2);
        reverse(adt, ad2);
    } else
    if(adt >= ad2) {
        dot = adt++;
        reverse(ad1, ad2);
        reverse(ad2, adt);
        reverse(ad1, adt);
    } else
        error(Q);
    fchange = true;
}
/*e: function [[move]](ed.c) */
/*s: function [[reverse]](ed.c) */
void
reverse(int *a1, int *a2)
{
    int t;

    for(;;) {
        t = *--a2;
        if(a2 <= a1)
            return;
        *a2 = *a1;
        *a1++ = t;
    }
}
/*e: function [[reverse]](ed.c) */
/*s: function [[getcopy]](ed.c) */
int
getcopy(void)
{
    if(addr1 > addr2)
        return EOF;
    getline(*addr1++);
    return 0;
}
/*e: function [[getcopy]](ed.c) */

/*s: function [[compile]](ed.c) */
/// commands('g' | 'v') -> global -> <>
void
compile(int eof)
{
    Rune c;
    char *ep;
    char expbuf[ESIZE];

    if((c = getchr()) == '\n') {
        peekc = c;
        c = eof;
    }
    if(c == eof) {
        if(!pattern)
            error(Q);
        return;
    }
    // else
    if(pattern) {
        free(pattern);
        pattern = nil;
    }
    ep = expbuf;
    do {
        if(c == '\\') {
            /*s: [[compile()]](ed.c) sanity check [[ep]] inside [[expbuf]] */
            if(ep >= expbuf+sizeof(expbuf)) {
                error(Q);
                return;
            }
            /*e: [[compile()]](ed.c) sanity check [[ep]] inside [[expbuf]] */
            // else
            ep += runetochar(ep, &c);
            if((c = getchr()) == '\n') {
                error(Q);
                return;
            }
        }
        /*s: [[compile()]](ed.c) sanity check [[ep]] inside [[expbuf]] */
        if(ep >= expbuf+sizeof(expbuf)) {
            error(Q);
            return;
        }
        /*e: [[compile()]](ed.c) sanity check [[ep]] inside [[expbuf]] */
        // else
        ep += runetochar(ep, &c);
    } while((c = getchr()) != eof && c != '\n');
    if(c == '\n')
        peekc = c;
    *ep = '\0';

    // lib_regexp call
    pattern = regcomp(expbuf);
}
/*e: function [[compile]](ed.c) */
/*s: function [[match]](ed.c) */
bool
match(int *addr)
{
    /*s: [[match()]](ed.c) return if no [[pattern]] */
    if(!pattern)
        return false;
    /*e: [[match()]](ed.c) return if no [[pattern]] */
    if(addr){
        /*s: [[match()]](ed.c) return if [[addr]] is [[zero]] */
        if(addr == zero)
            return false;
        /*e: [[match()]](ed.c) return if [[addr]] is [[zero]] */
        subexp[0].s.rsp = getline(*addr);
    } else
        /*s: [[match()]](ed.c) when null [[addr]] use [[loc2]] not [[getline]] */
        subexp[0].s.rsp = loc2;
        /*e: [[match()]](ed.c) when null [[addr]] use [[loc2]] not [[getline]] */
    subexp[0].e.rep = nil;

    if(rregexec(pattern, linebuf, subexp, MAXSUB)) {
        /*s: [[match()]](ed.c) set [[loc1]] and [[loc2]] with matched string */
        loc1 = subexp[0].s.rsp;
        loc2 = subexp[0].e.rep;
        /*e: [[match()]](ed.c) set [[loc1]] and [[loc2]] with matched string */
        return true;
    }
    // else
    /*s: [[match()]](ed.c) reset [[loc1]] and [[loc2]] */
    loc1 = loc2 = nil;
    /*e: [[match()]](ed.c) reset [[loc1]] and [[loc2]] */
    return false;
}
/*e: function [[match]](ed.c) */

// Printing text
/*s: function [[putd]](ex.c) */
void
putd(void)
{
    int r;

    r = count%10;
    count /= 10;
    if(count)
        putd();
    putchr(r + L'0');
}
/*e: function [[putd]](ex.c) */
/*s: function [[putstr]](ed.c) */
/// commands | getfile | error_1 | ... -> <> 
void
putst(char *sp)
{
    Rune r;

    col = 0;
    for(;;) {
        sp += chartorune(&r, sp);
        if(r == 0)
            break;
        putchr(r);
    }
    putchr(L'\n');
}
/*e: function [[putstr]](ed.c) */
/*s: function [[putshst]](ed.c) */
void
putshst(Rune *sp)
{
    col = 0;
    while(*sp)
        putchr(*sp++);
    putchr(L'\n');
}
/*e: function [[putshst]](ed.c) */
/*s: function [[putchr]](ed.c) */
/// error | putst | ... -> <>
void
putchr(int ac)
{
    char *lp;
    int c;
    Rune rune;

    lp = linp;
    c = ac;
    /*s: [[putchr()]] if [[listf]] */
    if(listf) {
        if(c == '\n') {
            if(linp != line && linp[-1] == ' ') {
                *lp++ = '\\';
                *lp++ = 'n';
            }
        } else {
            if(col > (72-6-2)) {
                col = 8;
                *lp++ = '\\';
                *lp++ = '\n';
                *lp++ = '\t';
            }
            col++;
            if(c=='\b' || c=='\t' || c=='\\') {
                *lp++ = '\\';
                if(c == '\b')
                    c = 'b';
                else
                if(c == '\t')
                    c = 't';
                col++;
            } else
            if(c<' ' || c>='\177') {
                *lp++ = '\\';
                *lp++ = 'x';
                *lp++ =  hex[c>>12];
                *lp++ =  hex[c>>8&0xF];
                *lp++ =  hex[c>>4&0xF];
                c     =  hex[c&0xF];
                col += 5;
            }
        }
    }
    /*e: [[putchr()]] if [[listf]] */

    rune = c;
    lp += runetochar(lp, &rune);

    if(c == '\n' || lp >= &line[sizeof(line)-5]) {
        linp = line;
        write(oflag? STDERR: STDOUT, line, lp-line);
        return;
    }
    // else
    linp = lp;
}
/*e: function [[putchr]](ed.c) */

/*s: function [[mktemp]](ed.c) */
/// main -> <>
char*
mktemp(char *as)
{
    char *s;
    unsigned pid;
    int i;

    pid = getpid();
    s = as;
    while(*s++)
        ;
    s--;

    while(*--s == 'X') {
        *s = pid % 10 + '0';
        pid /= 10;
    }
    s++;

    i = 'a';
    while(access(as, 0) != -1) {
        if(i == 'z')
            return "/";
        *s = i++;
    }
    return as;
}
/*e: function [[mktemp]](ed.c) */

/*s: function [[regerror]](ed.c) */
void
regerror(char *s)
{
    USED(s);
    error(Q);
}
/*e: function [[regerror]](ed.c) */
/*e: ed/ed.c */
