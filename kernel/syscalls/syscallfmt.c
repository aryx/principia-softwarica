/*s: syscallfmt.c */
/*
 * Print functions for system call tracing.
 */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "../port/systab.h"

/*s: function [[fmtrwdata]] */
// WE ARE OVERRUNNING SOMEHOW
static void
fmtrwdata(Fmt* f, char* a, int n, char* suffix)
{
    int i;
    char *t;

    if(a == nil){
        fmtprint(f, "0x0%s", suffix);
        return;
    }
    validaddr((ulong)a, n, false);
    t = smalloc(n+1);
    for(i = 0; i < n; i++)
        if(a[i] > 0x20 && a[i] < 0x7f)  /* printable ascii? */
            t[i] = a[i];
        else
            t[i] = '.';

    fmtprint(f, " %#p/\"%s\"%s", a, t, suffix);
    free(t);
}
/*e: function [[fmtrwdata]] */

/*s: function [[fmtuserstring]] */
static void
fmtuserstring(Fmt* f, char* a, char* suffix)
{
    int n;
    char *t;

    if(a == nil){
        fmtprint(f, "0/\"\"%s", suffix);
        return;
    }
    validaddr((ulong)a, 1, false);
    n = ((char*)vmemchr(a, 0, 0x7fffffff) - a) + 1;
    t = smalloc(n+1);
    memmove(t, a, n);
    t[n] = 0;
    fmtprint(f, "%#p/\"%s\"%s", a, t, suffix);
    free(t);
}
/*e: function [[fmtuserstring]] */

/*s: function [[syscallfmt]] */
void
syscallfmt(int syscallno, ulong pc, va_list list)
{
    long l;
    Fmt fmt;
    void *v;
    vlong vl;
    uintptr p;
    int i[2], len;
    char *a, **argv;

    fmtstrinit(&fmt);
    fmtprint(&fmt, "%uld %s ", up->pid, up->text);

    if(syscallno > nsyscall)
        fmtprint(&fmt, " %d ", syscallno);
    else
        fmtprint(&fmt, "%s ", sysctab[syscallno]?
            sysctab[syscallno]: "huh?");

    fmtprint(&fmt, "%ulx ", pc);
    if(up->syscalltrace != nil)
        free(up->syscalltrace);

    switch(syscallno){
    case NOP:
        p = va_arg(list, uintptr);
        fmtprint(&fmt, "%#p", p);
        break;
    case CHDIR:
    case EXITS:
    case REMOVE:
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, "");
        break;
    case BIND:
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        i[0] = va_arg(list, int);
        fmtprint(&fmt, "%#ux",  i[0]);
        break;
    case CLOSE:
    case NOTED:
        i[0] = va_arg(list, int);
        fmtprint(&fmt, "%d", i[0]);
        break;
    case DUP:
        i[0] = va_arg(list, int);
        i[1] = va_arg(list, int);
        fmtprint(&fmt, "%d %d", i[0], i[1]);
        break;
    case ALARM:
        l = va_arg(list, unsigned long);
        fmtprint(&fmt, "%#lud ", l);
        break;
    case EXEC:
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, "");
        argv = va_arg(list, char**);
        //validalign(PTR2UINT(argv), sizeof(char*)); ??
        for(;;){
            validaddr((ulong)argv, sizeof(char**), false);
            a = *(char **)argv;
            if(a == nil)
                break;
            fmtprint(&fmt, " ");
            fmtuserstring(&fmt, a, "");
            argv++;
        }
        break;
    case FAUTH:
        i[0] = va_arg(list, int);
        a = va_arg(list, char*);
        fmtprint(&fmt, "%d", i[0]);
        fmtuserstring(&fmt, a, "");
        break;
    case SEGBRK:
    case RENDEZVOUS:
        v = va_arg(list, void*);
        fmtprint(&fmt, "%#p ", v);
        v = va_arg(list, void*);
        fmtprint(&fmt, "%#p", v);
        break;
    case OPEN:
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        i[0] = va_arg(list, int);
        fmtprint(&fmt, "%#ux", i[0]);
        break;
    case SLEEP:
        l = va_arg(list, long);
        fmtprint(&fmt, "%ld", l);
        break;
    case RFORK:
        i[0] = va_arg(list, int);
        fmtprint(&fmt, "%#ux", i[0]);
        break;
    case PIPE:
    case BRK:
        v = va_arg(list, int*);
        fmtprint(&fmt, "%#p", v);
        break;
    case CREATE:
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        i[0] = va_arg(list, int);
        i[1] = va_arg(list, int);
        fmtprint(&fmt, "%#ux %#ux", i[0], i[1]);
        break;
    case FD2PATH:
    case FSTAT:
    case FWSTAT:
        i[0] = va_arg(list, int);
        a = va_arg(list, char*);
        l = va_arg(list, unsigned long);
        fmtprint(&fmt, "%d %#p %lud", i[0], a, l);
        break;
    case NOTIFY:
    case SEGDETACH:
        v = va_arg(list, void*);
        fmtprint(&fmt, "%#p", v);
        break;
    case SEGATTACH:
        i[0] = va_arg(list, int);
        fmtprint(&fmt, "%d ", i[0]);
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        /*FALLTHROUGH*/
    case SEGFREE:
    case SEGFLUSH:
        v = va_arg(list, void*);
        l = va_arg(list, unsigned long);
        fmtprint(&fmt, "%#p %lud", v, l);
        break;
    case UNMOUNT:
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, "");
        break;
    case SEMACQUIRE:
    case SEMRELEASE:
        v = va_arg(list, int*);
        i[0] = va_arg(list, int);
        fmtprint(&fmt, "%#p %d", v, i[0]);
        break;
    case TSEMACQUIRE:
        v = va_arg(list, long*);
        l = va_arg(list, ulong);
        fmtprint(&fmt, "%#p %ld", v, l);
        break;
    case SEEK:
        v = va_arg(list, vlong*);
        i[0] = va_arg(list, int);
        vl = va_arg(list, vlong);
        i[1] = va_arg(list, int);
        fmtprint(&fmt, "%#p %d %#llux %d", v, i[0], vl, i[1]);
        break;
    case FVERSION:
        i[0] = va_arg(list, int);
        i[1] = va_arg(list, int);
        fmtprint(&fmt, "%d %d ", i[0], i[1]);
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        l = va_arg(list, unsigned long);
        fmtprint(&fmt, "%lud", l);
        break;
    case WSTAT:
    case STAT:
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        /*FALLTHROUGH*/
    case ERRSTR:
    case AWAIT:
        a = va_arg(list, char*);
        l = va_arg(list, unsigned long);
        fmtprint(&fmt, "%#p %lud", a, l);
        break;
    case MOUNT:
        i[0] = va_arg(list, int);
        i[1] = va_arg(list, int);
        fmtprint(&fmt, "%d %d ", i[0], i[1]);
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, " ");
        i[0] = va_arg(list, int);
        fmtprint(&fmt, "%#ux ", i[0]);
        a = va_arg(list, char*);
        fmtuserstring(&fmt, a, "");
        break;
    case PREAD:
        i[0] = va_arg(list, int);
        v = va_arg(list, void*);
        l = va_arg(list, long);
        fmtprint(&fmt, "%d %#p %ld", i[0], v, l);
        if(syscallno == PREAD){
            vl = va_arg(list, vlong);
            fmtprint(&fmt, " %lld", vl);
        }
        break;
    case PWRITE:
        i[0] = va_arg(list, int);
        v = va_arg(list, void*);
        l = va_arg(list, long);
        fmtprint(&fmt, "%d ", i[0]);
        len = MIN(l, 64);
        fmtrwdata(&fmt, v, len, " ");
        fmtprint(&fmt, "%ld", l);
        if(syscallno == PWRITE){
            vl = va_arg(list, vlong);
            fmtprint(&fmt, " %lld", vl);
        }
        break;
    }

    up->syscalltrace = fmtstrflush(&fmt);
}
/*e: function [[syscallfmt]] */

/*s: function [[sysretfmt]] */
void
sysretfmt(int syscallno, va_list list, long ret, uvlong start, uvlong stop)
{
    long l;
    void* v;
    Fmt fmt;
    vlong vl;
    int i, len;
    char *a, *errstr;

    fmtstrinit(&fmt);

    if(up->syscalltrace)
        free(up->syscalltrace);

    errstr = "\"\"";
    switch(syscallno){
    default:
    case ALARM:
    case PWRITE:
        if(ret == -1)
            errstr = up->syserrstr;
        fmtprint(&fmt, " = %ld", ret);
        break;
    case EXEC:
    case SEGBRK:
    case SEGATTACH:
    case RENDEZVOUS:
        if((void *)ret == (void*)-1)
            errstr = up->syserrstr;
        fmtprint(&fmt, " = %#p", (void *)ret);
        break;
    case AWAIT:
        a = va_arg(list, char*);
        l = va_arg(list, unsigned long);
        if(ret > 0){
            fmtuserstring(&fmt, a, " ");
            fmtprint(&fmt, "%lud = %ld", l, ret);
        }
        else{
            fmtprint(&fmt, "%#p/\"\" %lud = %ld", a, l, ret);
            errstr = up->syserrstr;
        }
        break;
    case ERRSTR:
        a = va_arg(list, char*);
        l = va_arg(list, unsigned long);
        if(ret > 0){
            fmtuserstring(&fmt, a, " ");
            fmtprint(&fmt, "%lud = %ld", l, ret);
        }
        else{
            fmtprint(&fmt, "\"\" %lud = %ld", l, ret);
            errstr = up->syserrstr;
        }
        break;
    case FD2PATH:
        i = va_arg(list, int);
        USED(i);
        a = va_arg(list, char*);
        l = va_arg(list, unsigned long);
        if(ret > 0){
            fmtuserstring(&fmt, a, " ");
            fmtprint(&fmt, "%lud = %ld", l, ret);
        }
        else{
            fmtprint(&fmt, "\"\" %lud = %ld", l, ret);
            errstr = up->syserrstr;
        }
        break;
    case PREAD:
        i = va_arg(list, int);
        USED(i);
        v = va_arg(list, void*);
        l = va_arg(list, long);
        if(ret > 0){
            len = MIN(ret, 64);
            fmtrwdata(&fmt, v, len, "");
        }
        else{
            fmtprint(&fmt, "/\"\"");
            errstr = up->syserrstr;
        }
        fmtprint(&fmt, " %ld", l);
        if(syscallno == PREAD){
            vl = va_arg(list, vlong);
            fmtprint(&fmt, " %lld", vl);
        }
        fmtprint(&fmt, " = %ld", ret);
        break;
    }
    fmtprint(&fmt, " %s %#llud %#llud\n", errstr, start, stop);
    up->syscalltrace = fmtstrflush(&fmt);
}
/*e: function [[sysretfmt]] */
/*e: syscallfmt.c */
