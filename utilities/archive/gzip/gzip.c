/*s: archive/gzip/gzip.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>
#include <flate.h>
#include "gzip.h"

// forward decls
static  error0 gzipf(char*, bool);
static  int gzip(char*, long, int, Biobuf*);
static  int crcread(void *fd, void *buf, int n);
static  int gzwrite(void *bout, void *buf, int n);

/*s: global flags gzip.c */
// -v
static  bool verbose;
// -D
static  bool debug;
/*e: global flags gzip.c */
/*s: globals gzip.c */
static  Biobuf  bout;
// compression level, default = -6
static  int level;
/*x: globals gzip.c */
static  ulong   *crctab;
/*x: globals gzip.c */
static  ulong   crc;
static  bool    eof;
static  ulong   totr;
/*e: globals gzip.c */

/*s: function [[usage]](gzip.c) */
void
usage(void)
{
    fprint(STDERR, "usage: gzip [-vcD] [-1-9] [file ...]\n");
    exits("usage");
}
/*e: function [[usage]](gzip.c) */
/*s: function [[main]](gzip.c) */
void
main(int argc, char *argv[])
{
    int i;
    // enum<FlateError> (OK = 0) and then error0 (OK = 1)
    int ok; 
    // -c
    bool stdout = false;

    level = 6;
    ARGBEGIN{
    case 'v':
        verbose = true;
        break;
    case 'D':
        debug = true;
        break;
    case 'c':
        stdout = true;
        break;
    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        level = ARGC() - '0';
        break;
    default:
        usage();
        break;
    }ARGEND

    crctab = mkcrctab(GZCRCPOLY);
    ok = deflateinit();
    if(ok != FlateOk)
        sysfatal("deflateinit failed: %s", flateerr(ok));

    if(argc == 0){
        Binit(&bout, STDOUT, OWRITE);
        ok = gzip(nil, time(0), STDIN, &bout);
        Bterm(&bout);
    }else{
        ok = OK_1;
        for(i = 0; i < argc; i++)
            ok &= gzipf(argv[i], stdout);
    }
    exits(ok ? nil: "errors");
}
/*e: function [[main]](gzip.c) */

/*s: function [[gzipf]] */
static error0
gzipf(char *file, bool stdout)
{
    Dir *dir;
    char ofile[256], *f, *s;
    fdt ifd, ofd;
    int ok;

    ifd = open(file, OREAD);
    if(ifd < 0){
        fprint(STDERR, "gzip: can't open %s: %r\n", file);
        return ERROR_0;
    }
    dir = dirfstat(ifd);
    if(dir == nil){
        fprint(STDERR, "gzip: can't stat %s: %r\n", file);
        close(ifd);
        return ERROR_0;
    }
    if(dir->mode & DMDIR){
        fprint(STDERR, "gzip: can't compress a directory\n");
        close(ifd);
        free(dir);
        return ERROR_0;
    }

    if(stdout){
        ofd = STDOUT;
        strcpy(ofile, "<stdout>");
    }else{
        f = strrchr(file, '/');
        if(f != nil)
            f++;
        else
            f = file;
        s = strrchr(f, '.');
        if(s != nil && s != ofile && strcmp(s, ".tar") == ORD__EQ){
            *s = '\0';
            snprint(ofile, sizeof(ofile), "%s.tgz", f);
        }else
            snprint(ofile, sizeof(ofile), "%s.gz", f);
        ofd = create(ofile, OWRITE, 0666);
        if(ofd < 0){
            fprint(STDERR, "gzip: can't open %s: %r\n", ofile);
            close(ifd);
            return ERROR_0;
        }
    }

    if(verbose)
        fprint(STDERR, "compressing %s to %s\n", file, ofile);

    Binit(&bout, ofd, OWRITE);
    ok = gzip(file, dir->mtime, ifd, &bout);
    if(!ok || Bflush(&bout) < 0){
        fprint(STDERR, "gzip: error writing %s: %r\n", ofile);
        if(!stdout)
            remove(ofile);
    }
    Bterm(&bout);
    free(dir);
    close(ifd);
    close(ofd);
    return ok;
}
/*e: function [[gzipf]] */
/*s: function [[gzip]] */
static int
gzip(char *file, long mtime, fdt ifd, Biobuf *bout)
{
    int flags, err;

    flags = 0;
    Bputc(bout, GZMAGIC1);
    Bputc(bout, GZMAGIC2);
    Bputc(bout, GZDEFLATE);

    if(file != nil)
        flags |= GZFNAME;
    Bputc(bout, flags);

    Bputc(bout, mtime);
    Bputc(bout, mtime>>8);
    Bputc(bout, mtime>>16);
    Bputc(bout, mtime>>24);

    Bputc(bout, 0);
    Bputc(bout, GZOSINFERNO);

    if(flags & GZFNAME)
        Bwrite(bout, file, strlen(file)+1);

    crc = 0;
    eof = 0;
    totr = 0;
    err = deflate(bout, gzwrite, (void*)ifd, crcread, level, debug);
    if(err != FlateOk){
        fprint(STDERR, "gzip: deflate failed: %s\n", flateerr(err));
        return ERROR_0;
    }

    Bputc(bout, crc);
    Bputc(bout, crc>>8);
    Bputc(bout, crc>>16);
    Bputc(bout, crc>>24);

    Bputc(bout, totr);
    Bputc(bout, totr>>8);
    Bputc(bout, totr>>16);
    Bputc(bout, totr>>24);

    return OK_1;
}
/*e: function [[gzip]] */

/*s: function [[crcread]](gzip.c) */
static int
crcread(void *fd, void *buf, int n)
{
    int nr, m;

    nr = 0;
    for(; !eof && n > 0; n -= m){
        m = read((int)(uintptr)fd, (char*)buf+nr, n);
        if(m <= 0){
            eof = true;
            if(m < 0)
                return -1;
            break;
        }
        nr += m;
    }
    crc = blockcrc(crctab, crc, buf, nr);
    totr += nr;
    return nr;
}
/*e: function [[crcread]](gzip.c) */
/*s: function [[gzwrite]](gzip.c) */
static int
gzwrite(void *bout, void *buf, int n)
{
    if(n != Bwrite(bout, buf, n)){
        eof = true;
        return -1;
    }
    return n;
}
/*e: function [[gzwrite]](gzip.c) */
/*e: archive/gzip/gzip.c */
