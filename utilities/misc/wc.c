/*s: misc/wc.c */
/*
 * Count bytes within runes, if it fits in a uvlong, and other things.
 */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>

/*s: globals wc.c */
/* flags, per-file counts, and total counts */
static bool pline, pword, prune, pbadr, pchar;
static uvlong nline, nword, nrune, nbadr, nchar;
static uvlong tnline, tnword, tnrune, tnbadr, tnchar;
/*e: globals wc.c */

/*s: enum wc.c */
enum{Space, Word};
/*e: enum wc.c */

/*s: function [[wc]] */
static void
wc(Biobuf *bin)
{
    int where;
    long r;

    nline = 0;
    nword = 0;
    nrune = 0;
    nbadr = 0;
    where = Space;
    while ((long)(r = Bgetrune(bin)) >= 0) {
        nrune++;
        if(r == Runeerror) {
            nbadr++;
            continue;
        }
        if(r == '\n')
            nline++;
        if(where == Word){
            if(isspacerune(r))
                where = Space;
        }else
            if(isspacerune(r) == 0){
                where = Word;
                nword++;
            }
    }
    nchar = Boffset(bin);
    tnline += nline;
    tnword += nword;
    tnrune += nrune;
    tnbadr += nbadr;
    tnchar += nchar;
}
/*e: function [[wc]] */
/*s: function [[report]](wc.c) */
static void
report(uvlong nline, uvlong nword, uvlong nrune, uvlong nbadr, uvlong nchar, char *fname)
{
    char line[1024], *s, *e;

    s = line;
    e = line + sizeof line;
    line[0] = '\0';
    if(pline)
        s = seprint(s, e, " %7llud", nline);
    if(pword)
        s = seprint(s, e, " %7llud", nword);
    if(prune)
        s = seprint(s, e, " %7llud", nrune);
    if(pbadr)
        s = seprint(s, e, " %7llud", nbadr);
    if(pchar)
        s = seprint(s, e, " %7llud", nchar);
    if(fname != nil)
        seprint(s, e, " %s",   fname);
    print("%s\n", line+1);
}
/*e: function [[report]](wc.c) */
/*s: function [[main]](wc.c) */
void
main(int argc, char *argv[])
{
    char *sts = nil;
    Biobuf sin, *bin;
    int i;

    ARGBEGIN {
    case 'l': pline = true; break;
    case 'w': pword = true; break;
    case 'c': pchar = true; break;
    // plan9 new: -r for runes
    case 'r': prune = true; break;
    // ??
    case 'b': pbadr = true; break;
    default:
        fprint(STDERR, "Usage: %s [-lwrbc] [file ...]\n", argv0);
        exits("usage");
    } ARGEND
    if(pline || pword || prune || pbadr || pchar == false){
        // defaults
        pline = true;
        pword = true;
        pchar = true;
    }
    if(argc == 0){
        Binit(&sin, STDIN, OREAD);
        wc(&sin);
        report(nline, nword, nrune, nbadr, nchar, nil);
        Bterm(&sin);
    }else{
        for(i = 0; i < argc; i++){
            bin = Bopen(argv[i], OREAD);
            if(bin == nil){
                perror(argv[i]);
                sts = "can't open";
                continue;
            }
            wc(bin);
            report(nline, nword, nrune, nbadr, nchar, argv[i]);
            Bterm(bin);
        }
        if(argc>1)
            report(tnline, tnword, tnrune, tnbadr, tnchar, "total");
    }
    exits(sts);
}
/*e: function [[main]](wc.c) */
/*e: misc/wc.c */
