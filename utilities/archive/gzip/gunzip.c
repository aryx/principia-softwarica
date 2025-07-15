/*s: archive/gzip/gunzip.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>
#include <flate.h>
#include "gzip.h"

typedef struct  GZHead  GZHead;
/*s: struct [[GZHead]] */
struct GZHead
{
    ulong   mtime;
    char    *file;
};
/*e: struct [[GZHead]] */

// forward decls
static  int crcwrite(void *bout, void *buf, int n);
static  int     get1(Biobuf *b);
static  ulong   get4(Biobuf *b);
static  int gunzipf(char *file, bool stdout);
static  int gunzip(fdt ofd, char *ofile, Biobuf *bin);
static  void    header(Biobuf *bin, GZHead *h);
static  void    trailer(Biobuf *bin, long wlen);
static  void    error(char*, ...);

#pragma varargck    argpos  error   1

/*s: globals flags gunzip.c */
// -v
static  bool verbose;
// -D
static  bool debug;
// -t ??
static  bool table;
// -T ??
static  bool settimes;
/*e: globals flags gunzip.c */
/*s: globals gunzip.c */
static  char    *infile;
static  Biobuf  bin;
/*x: globals gunzip.c */
static  ulong   *crctab;
/*x: globals gunzip.c */
static  vlong   gzok;
static  jmp_buf zjmp;
static  char    *delfile;
/*x: globals gunzip.c */
static  ulong   crc;
static  ulong   wlen;
/*x: globals gunzip.c */
// error in writing (write is bad)
static  bool wbad;
/*e: globals gunzip.c */

/*s: function [[usage]](gunzip.c) */
void
usage(void)
{
    fprint(STDERR, "usage: gunzip [-ctvTD] [file ....]\n");
    exits("usage");
}
/*e: function [[usage]](gunzip.c) */
/*s: function [[main]](gunzip.c) */
void
main(int argc, char *argv[])
{
    int i;
    // enum<FlateError> (OK = 0) and then error0 (OK = 1, hmmm)
    int ok;
    // -c
    bool stdout;

    stdout = false;
    ARGBEGIN{
    case 'D':
        debug = true;
        break;
    case 'c':
        stdout = true;
        break;
    case 't':
        table = true;
        break;
    case 'T':
        settimes = true;
        break;
    case 'v':
        verbose = true;
        break;
    default:
        usage();
        break;
    }ARGEND

    crctab = mkcrctab(GZCRCPOLY);
    ok = inflateinit();
    if(ok != FlateOk)
        sysfatal("inflateinit failed: %s", flateerr(ok));

    if(argc == 0){
        Binit(&bin, STDIN, OREAD);
        settimes = false;
        infile = "<stdin>";
        ok = gunzip(STDOUT, "<stdout>", &bin);
    }else{
        ok = OK_1;
        if(stdout)
            settimes = false;
        for(i = 0; i < argc; i++)
            ok &= gunzipf(argv[i], stdout);
    }

    exits(ok ? nil: "errors");
}
/*e: function [[main]](gunzip.c) */

/*s: function [[gunzipf]] */
static error0
gunzipf(char *file, bool stdout)
{
    char ofile[256], *s;
    fdt ofd, ifd;
    int ok;

    infile = file;
    ifd = open(file, OREAD);
    if(ifd < 0){
        fprint(STDERR, "gunzip: can't open %s: %r\n", file);
        return ERROR_0;
    }

    Binit(&bin, ifd, OREAD);
    if(Bgetc(&bin) != GZMAGIC1 || Bgetc(&bin) != GZMAGIC2 || Bgetc(&bin) != GZDEFLATE){
        fprint(STDERR, "gunzip: %s is not a gzip deflate file\n", file);
        Bterm(&bin);
        close(ifd);
        return ERROR_0;
    }
    Bungetc(&bin);
    Bungetc(&bin);
    Bungetc(&bin);

    if(table)
        ofd = -1;
    else if(stdout){
        ofd = STDOUT;
        strcpy(ofile, "<stdout>");
    }else{
        s = strrchr(file, '/');
        if(s != nil)
            s++;
        else
            s = file;
        strecpy(ofile, ofile+sizeof ofile, s);
        s = strrchr(ofile, '.');
        if(s != nil && s != ofile && strcmp(s, ".gz") == ORD__EQ)
            *s = '\0';
        else if(s != nil && strcmp(s, ".tgz") == ORD__EQ)
            strcpy(s, ".tar");
        else if(strcmp(file, ofile) == ORD__EQ){
            fprint(STDERR, "gunzip: can't overwrite %s\n", file);
            Bterm(&bin);
            close(ifd);
            return ERROR_0;
        }

        ofd = create(ofile, OWRITE, 0666);
        if(ofd < 0){
            fprint(STDERR, "gunzip: can't create %s: %r\n", ofile);
            Bterm(&bin);
            close(ifd);
            return ERROR_0;
        }
        delfile = ofile;
    }

    wbad = false;
    // back go gunzip
    ok = gunzip(ofd, ofile, &bin);
    Bterm(&bin);
    close(ifd);
    if(wbad){
        fprint(STDERR, "gunzip: can't write %s: %r\n", ofile);
        if(delfile)
            remove(delfile);
    }
    delfile = nil;
    if(!stdout && ofd >= 0)
        close(ofd);
    return ok;
}
/*e: function [[gunzipf]] */
/*s: function [[gunzip]] */
static error0
gunzip(fdt ofd, char *ofile, Biobuf *bin)
{
    Dir *d;
    GZHead h;
    int err;

    h.file = nil;
    gzok = 0;
    for(;;){
        if(Bgetc(bin) < 0)
            return OK_1;
        // else
        Bungetc(bin);

        if(setjmp(zjmp))
            return ERROR_0;
        header(bin, &h);
        gzok = 0;

        wlen = 0;
        crc = 0;

        if(!table && verbose)
            fprint(STDERR, "extracting %s to %s\n", h.file, ofile);

        // call to libflate library
        err = inflate((void*)ofd, crcwrite, bin, (int(*)(void*))Bgetc);
        if(err != FlateOk)
            error("inflate failed: %s", flateerr(err));

        trailer(bin, wlen);

        if(table){
            if(verbose)
                print("%-32s %10ld %s", h.file, wlen, ctime(h.mtime));
            else
                print("%s\n", h.file);
        }else if(settimes && h.mtime && (d=dirfstat(ofd)) != nil){
            d->mtime = h.mtime;
            dirfwstat(ofd, d);
            free(d);
        }

        free(h.file);
        h.file = nil;
        gzok = Boffset(bin);
    }
}
/*e: function [[gunzip]] */

/*s: function [[header]](gunzip.c) */
static void
header(Biobuf *bin, GZHead *h)
{
    char *s;
    int i, c, flag, ns, nsa;

    if(get1(bin) != GZMAGIC1 || get1(bin) != GZMAGIC2)
        error("bad gzip file magic");
    if(get1(bin) != GZDEFLATE)
        error("unknown compression type");

    flag = get1(bin);
    if(flag & ~(GZFTEXT|GZFEXTRA|GZFNAME|GZFCOMMENT|GZFHCRC))
        fprint(STDERR, "gunzip: reserved flags set, data may not be decompressed correctly\n");

    /* mod time */
    h->mtime = get4(bin);

    /* extra flags */
    get1(bin);

    /* OS type */
    get1(bin);

    if(flag & GZFEXTRA)
        for(i=get1(bin); i>0; i--)
            get1(bin);

    /* name */
    if(flag & GZFNAME){
        nsa = 32;
        ns = 0;
        s = malloc(nsa);
        if(s == nil)
            error("out of memory");
        while((c = get1(bin)) != 0){
            s[ns++] = c;
            if(ns >= nsa){
                nsa += 32;
                s = realloc(s, nsa);
                if(s == nil)
                    error("out of memory");
            }
        }
        s[ns] = '\0';
        h->file = s;
    }else
        h->file = strdup("<unnamed file>");

    /* comment */
    if(flag & GZFCOMMENT)
        while(get1(bin) != 0)
            ;

    /* crc16 */
    if(flag & GZFHCRC){
        get1(bin);
        get1(bin);
    }
}
/*e: function [[header]](gunzip.c) */
/*s: function [[trailer]](gunzip.c) */
static void
trailer(Biobuf *bin, long wlen)
{
    ulong tcrc;
    long len;

    tcrc = get4(bin);
    if(tcrc != crc)
        error("crc mismatch");

    len = get4(bin);

    if(len != wlen)
        error("bad output length: expected %lud got %lud", wlen, len);
}
/*e: function [[trailer]](gunzip.c) */

/*s: function [[get4]](gunzip.c) */
static ulong
get4(Biobuf *b)
{
    ulong v;
    int i, c;

    v = 0;
    for(i = 0; i < 4; i++){
        c = Bgetc(b);
        if(c < 0)
            error("unexpected eof reading file information");
        v |= c << (i * 8);
    }
    return v;
}
/*e: function [[get4]](gunzip.c) */
/*s: function [[get1]](gunzip.c) */
static int
get1(Biobuf *b)
{
    int c;

    c = Bgetc(b);
    if(c < 0)
        error("unexpected eof reading file information");
    return c;
}
/*e: function [[get1]](gunzip.c) */
/*s: function [[crcwrite]](gunzip.c) */
static int
crcwrite(void *out, void *buf, int n)
{
    fdt fd;
    int nw;

    wlen += n;
    // blockcrc in libflate
    crc = blockcrc(crctab, crc, buf, n);
    fd = (int)(uintptr)out;
    if(fd < 0)
        return n;
    nw = write(fd, buf, n);
    if(nw != n)
        wbad = true;
    return nw;
}
/*e: function [[crcwrite]](gunzip.c) */

/*s: function [[error]](gunzip.c) */
static void
error(char *fmt, ...)
{
    va_list arg;

    if(gzok)
        fprint(STDERR, "gunzip: %s: corrupted data after byte %lld ignored\n", infile, gzok);
    else{
        fprint(STDERR, "gunzip: ");
        if(infile)
            fprint(STDERR, "%s: ", infile);
        va_start(arg, fmt);
        vfprint(STDERR, fmt, arg);
        va_end(arg);
        fprint(STDERR, "\n");
    
        if(delfile != nil){
            fprint(STDERR, "gunzip: removing output file %s\n", delfile);
            remove(delfile);
            delfile = nil;
        }
    }

    longjmp(zjmp, 1);
}
/*e: function [[error]](gunzip.c) */
/*e: archive/gzip/gunzip.c */
