/*s: files/rm.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: global [[errbuf]](rm.c) */
char errbuf[ERRMAX];
/*e: global [[errbuf]](rm.c) */
/*s: global [[ignerr]](rm.c) */
// for 'rm -f'
bool ignerr = false;
/*e: global [[ignerr]](rm.c) */

/*s: function [[err]](rm.c) */
void
err(char *f)
{
    if(!ignerr){
        errbuf[0] = '\0';
        errstr(errbuf, sizeof errbuf);
        fprint(STDERR, "rm: %s: %s\n", f, errbuf);
    }
}
/*e: function [[err]](rm.c) */
/*s: function [[rmdir]] */
/*
 * f is a non-empty directory. Remove its contents and then it.
 */
void
rmdir_(char *f)
{
    char *name;
    int i, j, n, ndir, nname;
    fdt fd;
    Dir *dirbuf;

    fd = open(f, OREAD);
    if(fd < 0){
        err(f);
        return;
    }
    //else
    n = dirreadall(fd, &dirbuf);
    close(fd);
    if(n < 0){
        err("dirreadall");
        return;
    }

    nname = strlen(f)+1+STATMAX+1;  /* plenty! */
    name = malloc(nname);
    if(name == nil){
        err("memory allocation");
        return;
    }

    ndir = 0;
    for(i=0; i<n; i++){
        snprint(name, nname, "%s/%s", f, dirbuf[i].name);
        if(remove(name) != ERROR_NEG1)
            dirbuf[i].qid.type = QTFILE;    /* so we won't recurse */
        else{
            if(dirbuf[i].qid.type & QTDIR)
                ndir++;
            else
                err(name);
        }
    }
    if(ndir)
        for(j=0; j<n; j++)
            if(dirbuf[j].qid.type & QTDIR){
                snprint(name, nname, "%s/%s", f, dirbuf[j].name);
                // recurse
                rmdir_(name);
            }
    if(remove(f) == ERROR_NEG1)
        err(f);
    free(name);
    free(dirbuf);
}
/*e: function [[rmdir]] */
/*s: function [[main]](rm.c) */
void
main(int argc, char *argv[])
{
    int i;
    // rm -r
    bool recurse = false;
    char *f;
    Dir *db;

    ARGBEGIN{
    case 'r':
        recurse = true;
        break;
    case 'f':
        ignerr = true;
        break;
    default:
        fprint(STDERR, "usage: rm [-fr] file ...\n");
        exits("usage");
    }ARGEND

    for(i=0; i<argc; i++){
        f = argv[i];
        if(remove(f) != ERROR_NEG1)
            continue;
        //else
        db = nil;
        if(recurse && (db=dirstat(f))!=nil && (db->qid.type&QTDIR))
            rmdir_(f);
        else
            err(f);
        free(db);
    }
    exits(errbuf);
}
/*e: function [[main]](rm.c) */
/*e: files/rm.c */
