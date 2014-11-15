/*s: lib_core/libthread/note.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: global _threadnopasser */
int	_threadnopasser;
/*e: global _threadnopasser */

/*s: constant NFN */
#define	NFN		33
/*e: constant NFN */
/*s: constant ERRLEN */
#define	ERRLEN	48
/*e: constant ERRLEN */
typedef struct Note Note;
/*s: struct Note */
struct Note
{
    Lock		inuse;
    Proc		*proc;		/* recipient */
    char		s[ERRMAX];	/* arg2 */
};
/*e: struct Note */

/*s: global notes */
static Note	notes[128];
/*e: global notes */
/*s: global enotes */
static Note	*enotes = notes+nelem(notes);
/*e: global enotes */
/*s: global onnote */
static int		(*onnote[NFN])(void*, char*);
/*e: global onnote */
/*s: global onnotepid */
static int		onnotepid[NFN];
/*e: global onnotepid */
/*s: global onnotelock */
static Lock	onnotelock;
/*e: global onnotelock */

/*s: function threadnotify */
int
threadnotify(int (*f)(void*, char*), int in)
{
    int i, topid;
    int (*from)(void*, char*), (*to)(void*, char*);

    if(in){
        from = nil;
        to = f;
        topid = _threadgetproc()->pid;
    }else{
        from = f;
        to = nil;
        topid = 0;
    }
    lock(&onnotelock);
    for(i=0; i<NFN; i++)
        if(onnote[i]==from){
            onnote[i] = to;
            onnotepid[i] = topid;
            break;
        }
    unlock(&onnotelock);
    return i<NFN;
}
/*e: function threadnotify */

/*s: function delayednotes */
static void
delayednotes(Proc *p, void *v)
{
    int i;
    Note *n;
    int (*fn)(void*, char*);

    if(!p->pending)
        return;

    p->pending = 0;
    for(n=notes; n<enotes; n++){
        if(n->proc == p){
            for(i=0; i<NFN; i++){
                if(onnotepid[i]!=p->pid || (fn = onnote[i])==nil)
                    continue;
                if((*fn)(v, n->s))
                    break;
            }
            if(i==NFN){
                _threaddebug(DBGNOTE, "Unhandled note %s, proc %p\n", n->s, p);
                if(v != nil)
                    noted(NDFLT);
                else if(strncmp(n->s, "sys:", 4)==0)
                    abort();
                threadexitsall(n->s);
            }
            n->proc = nil;
            unlock(&n->inuse);
        }
    }
}
/*e: function delayednotes */

/*s: function _threadnote */
void
_threadnote(void *v, char *s)
{
    Proc *p;
    Note *n;

    _threaddebug(DBGNOTE, "Got note %s", s);
    if(strncmp(s, "sys:", 4) == 0)
        noted(NDFLT);

    if(_threadexitsallstatus){
        _threaddebug(DBGNOTE, "Threadexitsallstatus = '%s'\n", _threadexitsallstatus);
        _exits(_threadexitsallstatus);
    }

    if(strcmp(s, "threadint")==0)
        noted(NCONT);

    p = _threadgetproc();
    if(p == nil)
        noted(NDFLT);

    for(n=notes; n<enotes; n++)
        if(canlock(&n->inuse))
            break;
    if(n==enotes)
        sysfatal("libthread: too many delayed notes");
    utfecpy(n->s, n->s+ERRMAX, s);
    n->proc = p;
    p->pending = 1;
    if(!p->splhi)
        delayednotes(p, v);
    noted(NCONT);
}
/*e: function _threadnote */

/*s: function _procsplhi */
int
_procsplhi(void)
{
    int s;
    Proc *p;

    p = _threadgetproc();
    s = p->splhi;
    p->splhi = 1;
    return s;
}
/*e: function _procsplhi */

/*s: function _procsplx */
void
_procsplx(int s)
{
    Proc *p;

    p = _threadgetproc();
    p->splhi = s;
    if(s)
        return;
    if(p->pending)
        delayednotes(p, nil);
}
/*e: function _procsplx */

/*e: lib_core/libthread/note.c */
