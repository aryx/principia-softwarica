/*s: git9/conf.c */
#include <u.h>
#include <libc.h>
#include <ctype.h>

#include "git.h"

/*s: global [[findroot]] */
bool findroot;
/*e: global [[findroot]] */
/*s: global [[showall]] */
bool showall;
/*e: global [[showall]] */
/*s: global [[nfile]] */
int nfile;
/*e: global [[nfile]] */
/*s: global [[file]] */
// array<ref_own<filename> (length=nfile)
char    *file[32];
/*e: global [[file]] */

/*s: function [[showconf]] */
static bool
showconf(char *cfg, char *sect, char *key)
{
    Biobuf *f;
    bool found, foundsect;
    char *ln, *p;
    int nsect, nkey;

    f = Bopen(cfg, OREAD);
    if(f == nil)
        return false;

    found = false;
    nsect = sect ? strlen(sect) : 0;
    nkey = strlen(key);
    foundsect = (sect == nil);

    while((ln = Brdstr(f, '\n', 1)) != nil){
        p = strip(ln);
        if(*p == '[' && sect){
            foundsect = strncmp(sect, ln, nsect) == 0;
        }else if(foundsect && strncmp(p, key, nkey) == 0){
            p = strip(p + nkey);
            if(*p != '=')
                continue;
            p = strip(p + 1);
            print("%s\n", p);
            found = true;
            if(!showall){
                free(ln);
                goto done;
            }
        }
        free(ln);
    }
done:
    return found;
}
/*e: function [[showconf]] */

/*s: function [[usage]] */
void
usage(void)
{
    fprint(STDERR, "usage: %s [-f file] [-r] keys..\n", argv0);
    fprint(STDERR, "\t-f:    use file 'file' (default: .git/config)\n");
    fprint(STDERR, "\t r:    print repository root\n");
    exits("usage");
}
/*e: function [[usage]] */
/*s: function [[main]] */
void
main(int argc, char **argv)
{
    char repo[512];
    int nrel;
    /*s: [[main()]](conf.c) other locals */
    char *p, *s;
    int i, j;
    /*e: [[main()]](conf.c) other locals */

    ARGBEGIN{
    /*s: [[main()]](conf.c) command line processing */
    case 'a':   showall=true;          break;
    /*x: [[main()]](conf.c) command line processing */
    case 'r':   findroot=true;         break;
    /*x: [[main()]](conf.c) command line processing */
    case 'f':
        if(nfile == nelem(file))
            sysfatal("too many configs");
        file[nfile++]=EARGF(usage());
        break;
    /*e: [[main()]](conf.c) command line processing */
    default:    usage();            break;
    }ARGEND;

    gitinit(repo, sizeof(repo), &nrel);

    /*s: [[main()]](conf.c) if [[findroot]] */
    if(findroot){
        print("%s\n", repo);
        exits(nil);
    }
    /*e: [[main()]](conf.c) if [[findroot]] */
    // else
    /*s: [[main()]](conf.c) set default [[file]] */
    if(nfile == 0){
        if((file[nfile++] = smprint("%s/.git/config", repo)) == nil)
            sysfatal("smprint: %r");
        if((p = getenv("home")) != nil)
            if((file[nfile++] = smprint("%s/lib/git/config", p)) == nil)
                sysfatal("smprint: %r");
        file[nfile++] = "/lib/git/config";
    }
    /*e: [[main()]](conf.c) set default [[file]] */

    for(i = 0; i < argc; i++){
        /*s: [[main()]](conf.c) show config entry for [[argv[i]]] */
        // syntax is git/fs <sect>.<key> or just <key>
        p = strchr(argv[i], '.');
        if(p == nil){
            s = nil;
            p = argv[i];
        }else{
            *p = '\0';
            p++;
            s = smprint("[%s]", argv[i]);
        }
        for(j = 0; j < nfile; j++)
            if(showconf(file[j], s, p))
                break;
        /*e: [[main()]](conf.c) show config entry for [[argv[i]]] */
    }
    exits(nil);
}
/*e: function [[main]] */
/*e: git9/conf.c */
