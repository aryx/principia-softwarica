/*s: git9/repack.c */
#include <u.h>
#include <libc.h>

#include "git.h"

/*s: macro [[TMPPATH]] */
#define TMPPATH(suff) (".git/objects/pack/repack."suff)
/*e: macro [[TMPPATH]] */

/*s: function [[cleanup]] */
int
cleanup(Hash h)
{
    char newpfx[42], dpath[256], fpath[256];
    int i, j, nd;
    Dir *d;

    snprint(newpfx, sizeof(newpfx), "%H.", h);
    for(i = 0; i < 256; i++){
        snprint(dpath, sizeof(dpath), ".git/objects/%02x", i);
        if((nd = slurpdir(dpath, &d)) == -1)
            continue;
        for(j = 0; j < nd; j++){
            snprint(fpath, sizeof(fpath), ".git/objects/%02x/%s", i, d[j].name);
            remove(fpath);
        }
        remove(dpath);
        free(d);
    }
    snprint(dpath, sizeof(dpath), ".git/objects/pack");
    if((nd = slurpdir(dpath, &d)) == -1)
        return -1;
    for(i = 0; i < nd; i++){
        if(strncmp(d[i].name, newpfx, strlen(newpfx)) == 0)
            continue;
        snprint(fpath, sizeof(fpath), ".git/objects/pack/%s", d[i].name);
        remove(fpath);
    }
    return 0;
}
/*e: function [[cleanup]] */

/*s: function [[usage (git9/repack.c)]] */
void
usage(void)
{
    fprint(STDERR, "usage: %s [-d]\n", argv0);
    exits("usage");
}
/*e: function [[usage (git9/repack.c)]] */
/*s: function [[main (git9/repack.c)]] */
void
main(int argc, char **argv)
{
    fdt fd;
    // growing_array<Hash> (len = nrefs)
    Hash *refs = nil;
    int nrefs;
    char **names;
    Hash h;
    Dir rn;
    char path[128];

    ARGBEGIN{
    /*s: [[main()]](repack.c) command line processing */
    case 'd':
        chattygit++;
        break;
    /*e: [[main()]](repack.c) command line processing */
    default: usage();
    }ARGEND;

    gitinit(nil, 0, nil);
    nrefs = listrefs(&refs, &names);
    /*s: [[main()]](repack.c) sanity check [[nrefs]] */
    if(nrefs == -1)
        sysfatal("load refs: %r");
    /*e: [[main()]](repack.c) sanity check [[nrefs]] */
    fd = create(TMPPATH("pack.tmp"), OWRITE, 0644);
    /*s: [[main()]](repack.c) sanity check [[fd]] */
    if(fd == -1)
        sysfatal("open %s: %r", TMPPATH("pack.tmp"));
    /*e: [[main()]](repack.c) sanity check [[fd]] */

    if(writepack(fd, refs, nrefs, nil, 0, &h) == -1)
        sysfatal("writepack: %r");
    if(indexpack(TMPPATH("pack.tmp"), TMPPATH("idx.tmp"), h) == ERROR_NEG1)
        sysfatal("indexpack: %r");
    close(fd);

    nulldir(&rn);
    rn.name = path;
    snprint(path, sizeof(path), "%H.pack", h);
    if(dirwstat(TMPPATH("pack.tmp"), &rn) == ERROR_NEG1)
        sysfatal("rename pack: %r");
    snprint(path, sizeof(path), "%H.idx", h);
    if(dirwstat(TMPPATH("idx.tmp"), &rn) == ERROR_NEG1)
        sysfatal("rename pack: %r");
    if(cleanup(h) == ERROR_NEG1)
        sysfatal("cleanup: %r");
    exits(nil);
}
/*e: function [[main (git9/repack.c)]] */
/*e: git9/repack.c */
