/*s: files/ls.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>
#ifdef Unix
#else
//DEAD include? remove the whole ifdef?
#include <fcall.h>
#endif

typedef struct NDir NDir;
/*s: struct [[NDir]] */
struct NDir
{
    // ref_own<Dir>
    Dir *d;
    // ?? for ls -lr ?
    char    *prefix;
};
/*e: struct [[NDir]] */

/*s: global flags ls.c */
// ??
bool dflag;
// ??
bool lflag;
// ??
bool mflag;
// ??
bool nflag;
// ??
bool pflag;
// ??
bool qflag;
// ??
bool Qflag;
// ??
bool rflag;
// ??
bool sflag;
// ??
bool tflag;
// ??
bool Tflag;
// ??
bool uflag;
// ??
bool Fflag;
/*e: global flags ls.c */
/*s: globals ls.c */
// growing_array<ref_own<NDir> (size = ndirbuf, valid max = ndir)
NDir*   dirbuf;
int ndirbuf;
int ndir;
/*x: globals ls.c */
Biobuf  bin;
/*x: globals ls.c */
bool errs = false;
/*x: globals ls.c */
int swidth;         /* max width of -s size */
int qwidth;         /* max width of -q version */
int vwidth;         /* max width of dev */
int uwidth;         /* max width of userid */
int mwidth;         /* max width of muid */
int lwidth;         /* max width of length */
int gwidth;         /* max width of groupid */
/*x: globals ls.c */
ulong   clk;
/*e: globals ls.c */

// forward decls
error1  ls(char*, bool);
ord     compar(const NDir*, const NDir*);
char*   asciitime(long);
char*   darwx(long);
void    rwx(long, char*);
void    growto(long);
void    dowidths(Dir*);
void    format(Dir*, char*);
void    output(void);
char*   xcleanname(char*);

/*s: function [[main]](ls.c) */
void
main(int argc, char *argv[])
{
    int i;

    Binit(&bin, STDOUT, OWRITE);
    ARGBEGIN{
    /*s: [[main]](ls.c) switch flag character cases */
    case 'F':   Fflag = true; break;
    case 'd':   dflag = true; break;
    case 'l':   lflag = true; break;
    case 'm':   mflag = true; break;
    case 'n':   nflag = true; break;
    case 'p':   pflag = true; break;
    case 'q':   qflag = true; break;
    case 'Q':   Qflag = true; break;
    case 'r':   rflag = true; break;
    case 's':   sflag = true; break;
    case 't':   tflag = true; break;
    case 'T':   Tflag = true; break;
    case 'u':   uflag = true; break;
    /*e: [[main]](ls.c) switch flag character cases */
    default:
       /*s: [[main]](ls.c) default case with usage */
       fprint(STDERR, "usage: ls [-dlmnpqrstuFQT] [file ...]\n");
       exits("usage");
       /*e: [[main]](ls.c) default case with usage */
    }ARGEND

    //XXX: doquote = needsrcquote;
    quotefmtinstall();
    //XXX: fmtinstall('M', dirmodefmt);

    /*s: [[main]](ls.c) if [[lflag]] */
    if(lflag)
        clk = time(0);
    /*e: [[main]](ls.c) if [[lflag]] */
    if(argc == 0)
        errs = ls(".", false);
    else for(i=0; i<argc; i++)
        errs |= ls(argv[i], true);

    output();
    exits(errs? "errors" : nil);
}
/*e: function [[main]](ls.c) */

/*s: function [[ls]] */
error1
ls(char *s, bool multi)
{
    fdt fd;
    long i, n;
    char *p;
    Dir *db;

    db = dirstat(s);
    if(db == nil){
    error:
        fprint(STDERR, "ls: %s: %r\n", s);
        return ERROR_1;
    }
    if((db->qid.type&QTDIR) && !dflag){
        free(db);
        output();
        fd = open(s, OREAD);
        if(fd == -1)
            goto error;
        n = dirreadall(fd, &db);
        if(n < 0)
            goto error;
        xcleanname(s);
        growto(ndir+n);
        for(i=0; i<n; i++){
            dirbuf[ndir+i].d = db+i;
            dirbuf[ndir+i].prefix = multi? s : nil;
        }
        ndir += n;
        close(fd);
        // why output() here? will be done in main() anyway
        output();
    }else{
        growto(ndir+1);
        dirbuf[ndir].d = db;
        dirbuf[ndir].prefix = nil;
        xcleanname(s);
        p = utfrrune(s, '/');
        if(p){
            dirbuf[ndir].prefix = s;
            *p = 0;
        }
        ndir++;
        // no output() ? will be done in main
    }
    return OK_0;
}
/*e: function [[ls]] */

/*s: function [[output]](ls.c) */
void
output(void)
{
    int i;
    char buf[4096];
    char *s;

    if(!nflag)
        qsort(dirbuf, ndir, sizeof dirbuf[0], (int (*)(const void*, const void*))compar);

    for(i=0; i<ndir; i++)
        dowidths(dirbuf[i].d);

    for(i=0; i<ndir; i++) {
        if(!pflag && (s = dirbuf[i].prefix)) {
            if(strcmp(s, "/") ==0)  /* / is a special case */
                s = "";
            sprint(buf, "%s/%s", s, dirbuf[i].d->name);
            format(dirbuf[i].d, buf);
        } else
            format(dirbuf[i].d, dirbuf[i].d->name);
    }
    ndir = 0;
    Bflush(&bin);
}
/*e: function [[output]](ls.c) */
/*s: function [[dowidths]](ls.c) */
void
dowidths(Dir *db)
{
    char buf[256];
    int n;

    if(sflag) {
        n = sprint(buf, "%llud", (db->length+1023)/1024);
        if(n > swidth)
            swidth = n;
    }
    if(qflag) {
        n = sprint(buf, "%lud", db->qid.vers);
        if(n > qwidth)
            qwidth = n;
    }
    if(mflag) {
        n = snprint(buf, sizeof buf, "[%q]", db->muid);
        if(n > mwidth)
            mwidth = n;
    }
    if(lflag) {
        n = sprint(buf, "%ud", db->dev);
        if(n > vwidth)
            vwidth = n;
        n = sprint(buf, "%q", db->uid);
        if(n > uwidth)
            uwidth = n;
        n = sprint(buf, "%q", db->gid);
        if(n > gwidth)
            gwidth = n;
        n = sprint(buf, "%llud", db->length);
        if(n > lwidth)
            lwidth = n;
    }
}
/*e: function [[dowidths]](ls.c) */
/*s: function [[fileflag]](ls.c) */
char*
fileflag(Dir *db)
{
    if(!Fflag)
        return "";
    if(QTDIR & db->qid.type)
        return "/";
    if(0111 & db->mode)
        return "*";
    return "";
}
/*e: function [[fileflag]](ls.c) */
/*s: function [[format]](ls.c) */
void
format(Dir *db, char *name)
{
    int i;

    if(sflag)
        Bprint(&bin, "%*llud ",
            swidth, (db->length+1023)/1024);
    if(mflag){
        Bprint(&bin, "[%q] ", db->muid);
        for(i=2+strlen(db->muid); i<mwidth; i++)
            Bprint(&bin, " ");
    }
    if(qflag)
        Bprint(&bin, "(%.16llux %*lud %.2ux) ",
            db->qid.path,
            qwidth, db->qid.vers,
            db->qid.type);
    if(Tflag)
        Bprint(&bin, "%c ", (db->mode&DMTMP)? 't': '-');

    if(lflag)
        Bprint(&bin, "%M %C %*ud %*q %*q %*llud %s ",
            db->mode, db->type,
            vwidth, db->dev,
            -uwidth, db->uid,
            -gwidth, db->gid,
            lwidth, db->length,
            asciitime(uflag? db->atime: db->mtime));

    Bprint(&bin, Qflag? "%s%s\n": "%q%s\n", name, fileflag(db));
}
/*e: function [[format]](ls.c) */

/*s: function [[growto]](ls.c) */
void
growto(long n)
{
    if(n <= ndirbuf)
        return;
    ndirbuf = n;
    dirbuf=(NDir *)realloc(dirbuf, ndirbuf*sizeof(NDir));
    if(dirbuf == nil){
        fprint(STDERR, "ls: malloc fail\n");
        exits("malloc fail");
    }
}
/*e: function [[growto]](ls.c) */
/*s: function [[compar]](ls.c) */
ord
compar(const NDir *a, const NDir *b)
{
    ord i;
    Dir *ad, *bd;

    ad = a->d;
    bd = b->d;

    if(tflag){
        if(uflag)
            i = bd->atime-ad->atime;
        else
            i = bd->mtime-ad->mtime;
    }else{
        if(a->prefix && b->prefix){
            i = strcmp(a->prefix, b->prefix);
            if(i == ORD__EQ)
                i = strcmp(ad->name, bd->name);
        }else if(a->prefix){
            i = strcmp(a->prefix, bd->name);
            if(i == ORD__EQ)
                i = ORD__SUP;  /* a is longer than b */
        }else if(b->prefix){
            i = strcmp(ad->name, b->prefix);
            if(i == ORD__EQ)
                i = ORD__INF; /* b is longer than a */
        }else
            i = strcmp(ad->name, bd->name);
    }
    if(i == ORD__EQ)
        i = (a<b? ORD__INF : ORD__SUP);
    if(rflag)
        i = -i;
    return i;
}
/*e: function [[compar]](ls.c) */
/*s: function [[asciitime]](ls.c) */
char*
asciitime(long l)
{
    static char buf[32];
    char *t;

    t = ctime(l);
    /* 6 months in the past or a day in the future */
    if(l<clk-180L*24*60*60 || clk+24L*60*60<l){
        memmove(buf, t+4, 7);       /* month and day */
        memmove(buf+7, t+23, 5);        /* year */
    }else
        memmove(buf, t+4, 12);      /* skip day of week */
    buf[12] = '\0';
    return buf;
}
/*e: function [[asciitime]](ls.c) */
/*s: function [[xcleanname]](ls.c) */
/*
 * Compress slashes, remove trailing slash.  Don't worry about . and ..
 */
char*
xcleanname(char *name)
{
    char *r, *w;

    for(r=w=name; *r; r++){
        if(*r=='/' && r>name && *(r-1)=='/')
            continue;
        if(w != r)
            *w = *r;
        w++;
    }
    *w = '\0';
    while(w-1>name && *(w-1)=='/')
        *--w = '\0';
    return name;
}
/*e: function [[xcleanname]](ls.c) */
/*e: files/ls.c */
