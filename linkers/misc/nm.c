/*s: linkers/misc/nm.c */
/*
 * nm.c -- drive nm
 */
#include <u.h>
#include <libc.h>
#include <ar.h>
#include <bio.h>
#include <mach.h>

/*s: enum _anon_ (linkers/misc/nm.c) */
enum{
    CHUNK	=	256	/* must be power of 2 */
};
/*e: enum _anon_ (linkers/misc/nm.c) */

/*s: global errs */
static char	*errs;			/* exit status */
/*e: global errs */
/*s: global filename */
static char	*filename;		/* current file */
/*e: global filename */
/*s: global symname */
static char	symname[]="__.SYMDEF";	/* table of contents file name */
/*e: global symname */
/*s: global multifile */
static int	multifile;		/* processing multiple files */
/*e: global multifile */
/*s: global aflag (linkers/misc/nm.c) */
static int	aflag;
/*e: global aflag (linkers/misc/nm.c) */
/*s: global gflag */
static int	gflag;
/*e: global gflag */
/*s: global hflag */
static int	hflag;
/*e: global hflag */
/*s: global nflag */
static int	nflag;
/*e: global nflag */
/*s: global sflag */
static int	sflag;
/*e: global sflag */
/*s: global uflag (linkers/misc/nm.c) */
static int	uflag;
/*e: global uflag (linkers/misc/nm.c) */
/*s: global Tflag */
static int	Tflag;
/*e: global Tflag */

/*s: global fnames */
static Sym	**fnames;		/* file path translation table */
/*e: global fnames */
/*s: global symptr */
static Sym	**symptr;
/*e: global symptr */
/*s: global nsym */
static int	nsym;
/*e: global nsym */
/*s: global bout (linkers/misc/nm.c) */
static Biobuf	bout;
/*e: global bout (linkers/misc/nm.c) */

int	cmp(void*, void*);
static void	error(char*, ...);
void	execsyms(int);
void	psym(Sym*, void*);
void	printsyms(Sym**, long);
void	doar(Biobuf*);
void	dofile(Biobuf*);
void	zenter(Sym*);

/*s: function usage (linkers/misc/nm.c) */
static void
usage(void)
{
    fprint(2, "usage: nm [-aghnsTu] file ...\n");
    exits("usage");
}
/*e: function usage (linkers/misc/nm.c) */

/*s: function main (linkers/misc/nm.c) */
void
main(int argc, char *argv[])
{
    int i;
    Biobuf	*bin;

    Binit(&bout, 1, OWRITE);
    argv0 = argv[0];
    ARGBEGIN {
    default:	usage();
    case 'a':	aflag = 1; break;
    case 'g':	gflag = 1; break;
    case 'h':	hflag = 1; break;
    case 'n':	nflag = 1; break;
    case 's':	sflag = 1; break;
    case 'u':	uflag = 1; break;
    case 'T':	Tflag = 1; break;
    } ARGEND
    if (argc == 0)
        usage();
    if (argc > 1)
        multifile++;
    for(i=0; i<argc; i++){
        filename = argv[i];
        bin = Bopen(filename, OREAD);
        if(bin == 0){
            error("cannot open %s", filename);
            continue;
        }
        if (isar(bin))
            doar(bin);
        else{
            Bseek(bin, 0, 0);
            dofile(bin);
        }
        Bterm(bin);
    }
    exits(errs);
}
/*e: function main (linkers/misc/nm.c) */

/*s: function doar */
/*
 * read an archive file,
 * processing the symbols for each intermediate file in it.
 */
void
doar(Biobuf *bp)
{
    int offset, size, obj;
    char membername[SARNAME];

    multifile = 1;
    for (offset = Boffset(bp);;offset += size) {
        size = nextar(bp, offset, membername);
        if (size < 0) {
            error("phase error on ar header %ld", offset);
            return;
        }
        if (size == 0)
            return;
        if (strcmp(membername, symname) == 0)
            continue;
        obj = objtype(bp, 0);
        if (obj < 0) {
            error("inconsistent file %s in %s",
                    membername, filename);
            return;
        }
        if (!readar(bp, obj, offset+size, 1)) {
            error("invalid symbol reference in file %s",
                    membername);
            return;
        }
        filename = membername;
        nsym=0;
        objtraverse(psym, 0);
        printsyms(symptr, nsym);
    }
}
/*e: function doar */

/*s: function dofile */
/*
 * process symbols in a file
 */
void
dofile(Biobuf *bp)
{
    int obj;

    obj = objtype(bp, 0);
    if (obj < 0)
        execsyms(Bfildes(bp));
    else
    if (readobj(bp, obj)) {
        nsym = 0;
        objtraverse(psym, 0);
        printsyms(symptr, nsym);
    }
}
/*e: function dofile */

/*s: function cmp */
/*
 * comparison routine for sorting the symbol table
 *	this screws up on 'z' records when aflag == 1
 */
int
cmp(void *vs, void *vt)
{
    Sym **s, **t;

    s = vs;
    t = vt;
    if(nflag)
        if((*s)->value < (*t)->value)
            return -1;
        else
            return (*s)->value > (*t)->value;
    return strcmp((*s)->name, (*t)->name);
}
/*e: function cmp */
/*s: function zenter */
/*
 * enter a symbol in the table of filename elements
 */
void
zenter(Sym *s)
{
    static int maxf = 0;

    if (s->value > maxf) {
        maxf = (s->value+CHUNK-1) &~ (CHUNK-1);
        fnames = realloc(fnames, (maxf+1)*sizeof(*fnames));
        if(fnames == 0) {
            error("out of memory", argv0);
            exits("memory");
        }
    }
    fnames[s->value] = s;
}
/*e: function zenter */

/*s: function execsyms */
/*
 * get the symbol table from an executable file, if it has one
 */
void
execsyms(int fd)
{
    Fhdr f;
    Sym *s;
    long n;

    seek(fd, 0, 0);
    if (crackhdr(fd, &f) == 0) {
        error("Can't read header for %s", filename);
        return;
    }
    if (syminit(fd, &f) < 0)
        return;
    s = symbase(&n);
    nsym = 0;
    while(n--)
        psym(s++, 0);

    printsyms(symptr, nsym);
}
/*e: function execsyms */

/*s: function psym */
void
psym(Sym *s, void* p)
{
    USED(p);
    switch(s->type) {
    case 'T':
    case 'L':
    case 'D':
    case 'B':
        if (uflag)
            return;
        if (!aflag && ((s->name[0] == '.' || s->name[0] == '$')))
            return;
        break;
    case 'b':
    case 'd':
    case 'l':
    case 't':
        if (uflag || gflag)
            return;
        if (!aflag && ((s->name[0] == '.' || s->name[0] == '$')))
            return;
        break;
    case 'U':
        if (gflag)
            return;
        break;
    case 'Z':
        if (!aflag)
            return;
        break;
    case 'm':
    case 'f':	/* we only see a 'z' when the following is true*/
        if(!aflag || uflag || gflag)
            return;
        if (strcmp(s->name, ".frame"))
            zenter(s);
        break;
    case 'a':
    case 'p':
    case 'z':
    default:
        if(!aflag || uflag || gflag)
            return;
        break;
    }
    symptr = realloc(symptr, (nsym+1)*sizeof(Sym*));
    if (symptr == 0) {
        error("out of memory");
        exits("memory");
    }
    symptr[nsym++] = s;
}
/*e: function psym */

/*s: function printsyms */
void
printsyms(Sym **symptr, long nsym)
{
    int i, wid;
    Sym *s;
    char *cp;
    char path[512];

    if(!sflag)
        qsort(symptr, nsym, sizeof(*symptr), cmp);
    
    wid = 0;
    for (i=0; i<nsym; i++) {
        s = symptr[i];
        if (s->value && wid == 0)
            wid = 8;
        else if (s->value >= 0x100000000LL && wid == 8)
            wid = 16;
    }	
    for (i=0; i<nsym; i++) {
        s = symptr[i];
        if (multifile && !hflag)
            Bprint(&bout, "%s:", filename);
        if (s->type == 'z') {
            fileelem(fnames, (uchar *) s->name, path, 512);
            cp = path;
        } else
            cp = s->name;
        if (Tflag)
            Bprint(&bout, "%8ux ", s->sig);
        if (s->value || s->type == 'a' || s->type == 'p')
            Bprint(&bout, "%*llux ", wid, s->value);
        else
            Bprint(&bout, "%*s ", wid, "");
        Bprint(&bout, "%c %s\n", s->type, cp);
    }
}
/*e: function printsyms */

/*s: function error */
static void
error(char *fmt, ...)
{
    Fmt f;
    char buf[128];
    va_list arg;

    fmtfdinit(&f, 2, buf, sizeof buf);
    fmtprint(&f, "%s: ", argv0);
    va_start(arg, fmt);
    fmtvprint(&f, fmt, arg);
    va_end(arg);
    fmtprint(&f, "\n");
    fmtfdflush(&f);
    errs = "errors";
}
/*e: function error */
/*e: linkers/misc/nm.c */
