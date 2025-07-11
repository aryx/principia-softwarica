/*s: files/chmod.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: macros chmod.c */
#define U(x) (x<<6)
#define G(x) (x<<3)
#define O(x) (x)
#define A(x) (U(x)|G(x)|O(x))

#define DMRWE (DMREAD|DMWRITE|DMEXEC)
/*e: macros chmod.c */

// forward decls
error0 parsemode(char *, ulong *, ulong *);

/*s: function [[main]](chmod.c) */
void
main(int argc, char *argv[])
{
    int i;
    Dir *dir, ndir;
    ulong mode, mask;
    char *p;

    if(argc < 3){
        fprint(STDERR, "usage: chmod 0777 file ... or chmod [who]op[rwxalt] file ...\n");
        exits("usage");
    }
    mode = strtol(argv[1], &p, 8);
    if(*p == '\0')
        mask = A(DMRWE);
    else if(parsemode(argv[1], &mask, &mode) == ERROR_0){
        fprint(STDERR, "chmod: bad mode: %s\n", argv[1]);
        exits("mode");
    }
    nulldir(&ndir);
    for(i=2; i<argc; i++){
        dir = dirstat(argv[i]);
        if(dir == nil){
            fprint(STDERR, "chmod: can't stat %s: %r\n", argv[i]);
            continue;
        }
        ndir.mode = (dir->mode & ~mask) | (mode & mask);
        free(dir);
        if(dirwstat(argv[i], &ndir)==-1){
            fprint(STDERR, "chmod: can't wstat %s: %r\n", argv[i]);
            continue;
        }
    }
    exits(nil);
}
/*e: function [[main]](chmod.c) */

/*s: function [[parsemode]](chmod.c) */
error0
parsemode(char *spec, ulong *pmask, ulong *pmode)
{
    ulong mode, mask;
    bool done;
    int op;
    char *s;

    s = spec;
    mask = DMAPPEND | DMEXCL | DMTMP;
    for(done=false; !done; ){
        switch(*s){
        case 'u':
            mask |= U(DMRWE); break;
        case 'g':
            mask |= G(DMRWE); break;
        case 'o':
            mask |= O(DMRWE); break;
        case 'a':
            mask |= A(DMRWE); break;
        case 0:
            return ERROR_0;
        default:
            done = true;
        }
        if(!done)
            s++;
    }
    if(s == spec)
        mask |= A(DMRWE);
    op = *s++;
    if(op != '+' && op != '-' && op != '=')
        return ERROR_0;
    mode = 0;
    for(; *s ; s++){
        switch(*s){
        case 'r':
            mode |= A(DMREAD); break;
        case 'w':
            mode |= A(DMWRITE); break;
        case 'x':
            mode |= A(DMEXEC); break;
        case 'a':
            mode |= DMAPPEND; break;
        case 'l':
            mode |= DMEXCL; break;
        case 't':
            mode |= DMTMP; break;
        default:
            return ERROR_0;
        }
    }
    if(*s != 0)
        return ERROR_0;
    if(op == '+' || op == '-')
        mask &= mode;
    if(op == '-')
        mode = ~mode;
    *pmask = mask;
    *pmode = mode;
    return OK_1;
}
/*e: function [[parsemode]](chmod.c) */
/*e: files/chmod.c */
