/*s: hoc/code.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include "hoc.h"
#include "y.tab.h"

/*s: constant [[NSTACK]](hoc) */
#define NSTACK  256
/*e: constant [[NSTACK]](hoc) */

static Datum stack[NSTACK];     /* the stack */
static Datum *stackp;           /* next free spot on stack */

/*s: constant [[NPROG]](hoc) */
#define NPROG   2000
/*e: constant [[NPROG]](hoc) */
Inst    prog[NPROG];    /* the machine */
Inst    *progp;         /* next free spot for code generation */
Inst    *pc;            /* program counter during execution */
Inst    *progbase = prog; /* start of current subprogram */
int     returning;      /* 1 if return stmt seen */
int     indef;  /* 1 if parsing a func or proc */

typedef struct Frame {  /* proc/func call stack frame */
    Symbol      *sp;    /* symbol table entry */
    Inst        *retpc; /* where to resume after return */
    Datum       *argn;  /* n-th argument on stack */
    int nargs;  /* number of arguments */
} Frame;
/*s: constant [[NFRAME]](hoc) */
#define NFRAME  100
/*e: constant [[NFRAME]](hoc) */
Frame   frame[NFRAME];
Frame   *fp;            /* frame pointer */

/*s: function [[initcode]](hoc) */
void
initcode(void)
{
    progp = progbase;
    stackp = stack;
    fp = frame;
    returning = 0;
    indef = 0;
}
/*e: function [[initcode]](hoc) */

/*s: function [[push]](hoc) */
void
push(Datum d)
{
    if (stackp >= &stack[NSTACK])
        execerror("stack too deep", 0);
    *stackp++ = d;
}
/*e: function [[push]](hoc) */

/*s: function [[pop]](hoc) */
Datum
pop(void)
{
    if (stackp == stack)
        execerror("stack underflow", 0);
    return *--stackp;
}
/*e: function [[pop]](hoc) */

/*s: function [[xpop]](hoc) */
void
xpop(void)      /* for when no value is wanted */
{
    if (stackp == stack)
        execerror("stack underflow", (char *)0);
    --stackp;
}
/*e: function [[xpop]](hoc) */

/*s: function [[constpush]](hoc) */
void
constpush(void)
{
    Datum d;
    d.val = ((Symbol *)*pc++)->u.val;
    push(d);
}
/*e: function [[constpush]](hoc) */

/*s: function [[varpush]](hoc) */
void
varpush(void)
{
    Datum d;
    d.sym = (Symbol *)(*pc++);
    push(d);
}
/*e: function [[varpush]](hoc) */

/*s: function [[whilecode]](hoc) */
void
whilecode(void)
{
    Datum d;
    Inst *savepc = pc;

    execute(savepc+2);  /* condition */
    d = pop();
    while (d.val) {
        execute(*((Inst **)(savepc)));  /* body */
        if (returning)
            break;
        execute(savepc+2);      /* condition */
        d = pop();
    }
    if (!returning)
        pc = *((Inst **)(savepc+1)); /* next stmt */
}
/*e: function [[whilecode]](hoc) */

/*s: function [[forcode]](hoc) */
void
forcode(void)
{
    Datum d;
    Inst *savepc = pc;

    execute(savepc+4);          /* precharge */
    pop();
    execute(*((Inst **)(savepc)));      /* condition */
    d = pop();
    while (d.val) {
        execute(*((Inst **)(savepc+2)));        /* body */
        if (returning)
            break;
        execute(*((Inst **)(savepc+1)));        /* post loop */
        pop();
        execute(*((Inst **)(savepc)));  /* condition */
        d = pop();
    }
    if (!returning)
        pc = *((Inst **)(savepc+3)); /* next stmt */
}
/*e: function [[forcode]](hoc) */

/*s: function [[ifcode]](hoc) */
void
ifcode(void) 
{
    Datum d;
    Inst *savepc = pc;  /* then part */

    execute(savepc+3);  /* condition */
    d = pop();
    if (d.val)
        execute(*((Inst **)(savepc)));  
    else if (*((Inst **)(savepc+1))) /* else part? */
        execute(*((Inst **)(savepc+1)));
    if (!returning)
        pc = *((Inst **)(savepc+2)); /* next stmt */
}
/*e: function [[ifcode]](hoc) */

/*s: function [[define]](hoc) */
void
define(Symbol* sp, Formal *f)   /* put func/proc in symbol table */
{
    Fndefn *fd;
    int n;

    fd = emalloc(sizeof(Fndefn));
    fd->code = progbase;        /* start of code */
    progbase = progp;   /* next code starts here */
    fd->formals = f;
    for(n=0; f; f=f->next)
        n++;
    fd->nargs = n;
    sp->u.defn = fd;
}
/*e: function [[define]](hoc) */

/*s: function [[call]](hoc) */
void
call(void)              /* call a function */
{
    Formal *f;
    Datum *arg;
    Saveval *s;
    int i;

    Symbol *sp = (Symbol *)pc[0]; /* symbol table entry */
                      /* for function */
    if (fp >= &frame[NFRAME-1])
        execerror(sp->name, "call nested too deeply");
    fp++;
    fp->sp = sp;
    fp->nargs = (int)(uintptr)pc[1];
    fp->retpc = pc + 2;
    fp->argn = stackp - 1;      /* last argument */
    if(fp->nargs != sp->u.defn->nargs)
        execerror(sp->name, "called with wrong number of arguments");
    /* bind formals */
    f = sp->u.defn->formals;
    arg = stackp - fp->nargs;
    while(f){
        s = emalloc(sizeof(Saveval));
        s->val = f->sym->u;
        s->type = f->sym->type;
        s->next = f->save;
        f->save = s;
        f->sym->u.val = arg->val;
        f->sym->type = VAR;
        f = f->next;
        arg++;
    }
    for (i = 0; i < fp->nargs; i++)
        pop();  /* pop arguments; no longer needed */
    execute(sp->u.defn->code);
    returning = 0;
}
/*e: function [[call]](hoc) */

/*s: function [[restore]](hoc) */
void
restore(Symbol *sp)     /* restore formals associated with symbol */
{
    Formal *f;
    Saveval *s;

    f = sp->u.defn->formals;
    while(f){
        s = f->save;
        if(s == 0)      /* more actuals than formals */
            break;
        f->sym->u = s->val;
        f->sym->type = s->type;
        f->save = s->next;
        free(s);
        f = f->next;
    }
}
/*e: function [[restore]](hoc) */

/*s: function [[restoreall]](hoc) */
void
restoreall(void)        /* restore all variables in case of error */
{
    while(fp>=frame && fp->sp){
        restore(fp->sp);
        --fp;
    }
    fp = frame;
}
/*e: function [[restoreall]](hoc) */

/*s: function [[ret]](hoc) */
static void
ret(void)               /* common return from func or proc */
{
    /* restore formals */
    restore(fp->sp);
    pc = (Inst *)fp->retpc;
    --fp;
    returning = 1;
}
/*e: function [[ret]](hoc) */

/*s: function [[funcret]](hoc) */
void
funcret(void)   /* return from a function */
{
    Datum d;
    if (fp->sp->type == PROCEDURE)
        execerror(fp->sp->name, "(proc) returns value");
    d = pop();  /* preserve function return value */
    ret();
    push(d);
}
/*e: function [[funcret]](hoc) */

/*s: function [[procret]](hoc) */
void
procret(void)   /* return from a procedure */
{
    if (fp->sp->type == FUNCTION)
        execerror(fp->sp->name,
            "(func) returns no value");
    ret();
}
/*e: function [[procret]](hoc) */

/*s: function [[bltin]](hoc) */
void
bltin(void) 
{

    Datum d;
    d = pop();
    d.val = (*(double (*)(double))*pc++)(d.val);
    push(d);
}
/*e: function [[bltin]](hoc) */

/*s: function [[add]](hoc) */
void
add(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val += d2.val;
    push(d1);
}
/*e: function [[add]](hoc) */

/*s: function [[sub]](hoc) */
void
sub(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val -= d2.val;
    push(d1);
}
/*e: function [[sub]](hoc) */

/*s: function [[mul]](hoc) */
void
mul(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val *= d2.val;
    push(d1);
}
/*e: function [[mul]](hoc) */

/*s: function [[div]](hoc) */
void
div(void)
{
    Datum d1, d2;
    d2 = pop();
    if (d2.val == 0.0)
        execerror("division by zero", (char *)0);
    d1 = pop();
    d1.val /= d2.val;
    push(d1);
}
/*e: function [[div]](hoc) */

/*s: function [[mod]](hoc) */
void
mod(void)
{
    Datum d1, d2;
    d2 = pop();
    if (d2.val == 0.0)
        execerror("division by zero", (char *)0);
    d1 = pop();
    /* d1.val %= d2.val; */
    d1.val = fmod(d1.val, d2.val);
    push(d1);
}
/*e: function [[mod]](hoc) */

/*s: function [[negate]](hoc) */
void
negate(void)
{
    Datum d;
    d = pop();
    d.val = -d.val;
    push(d);
}
/*e: function [[negate]](hoc) */

/*s: function [[verify]](hoc) */
void
verify(Symbol* s)
{
    if (s->type != VAR && s->type != UNDEF)
        execerror("attempt to evaluate non-variable", s->name);
    if (s->type == UNDEF)
        execerror("undefined variable", s->name);
}
/*e: function [[verify]](hoc) */

/*s: function [[eval]](hoc) */
void
eval(void)              /* evaluate variable on stack */
{
    Datum d;
    d = pop();
    verify(d.sym);
    d.val = d.sym->u.val;
    push(d);
}
/*e: function [[eval]](hoc) */

/*s: function [[preinc]](hoc) */
void
preinc(void)
{
    Datum d;
    d.sym = (Symbol *)(*pc++);
    verify(d.sym);
    d.val = d.sym->u.val += 1.0;
    push(d);
}
/*e: function [[preinc]](hoc) */

/*s: function [[predec]](hoc) */
void
predec(void)
{
    Datum d;
    d.sym = (Symbol *)(*pc++);
    verify(d.sym);
    d.val = d.sym->u.val -= 1.0;
    push(d);
}
/*e: function [[predec]](hoc) */

/*s: function [[postinc]](hoc) */
void
postinc(void)
{
    Datum d;
    double v;
    d.sym = (Symbol *)(*pc++);
    verify(d.sym);
    v = d.sym->u.val;
    d.sym->u.val += 1.0;
    d.val = v;
    push(d);
}
/*e: function [[postinc]](hoc) */

/*s: function [[postdec]](hoc) */
void
postdec(void)
{
    Datum d;
    double v;
    d.sym = (Symbol *)(*pc++);
    verify(d.sym);
    v = d.sym->u.val;
    d.sym->u.val -= 1.0;
    d.val = v;
    push(d);
}
/*e: function [[postdec]](hoc) */

/*s: function [[gt]](hoc) */
void
gt(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val > d2.val);
    push(d1);
}
/*e: function [[gt]](hoc) */

/*s: function [[lt]](hoc) */
void
lt(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val < d2.val);
    push(d1);
}
/*e: function [[lt]](hoc) */

/*s: function [[ge]](hoc) */
void
ge(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val >= d2.val);
    push(d1);
}
/*e: function [[ge]](hoc) */

/*s: function [[le]](hoc) */
void
le(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val <= d2.val);
    push(d1);
}
/*e: function [[le]](hoc) */

/*s: function [[eq]](hoc) */
void
eq(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val == d2.val);
    push(d1);
}
/*e: function [[eq]](hoc) */

/*s: function [[ne]](hoc) */
void
ne(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val != d2.val);
    push(d1);
}
/*e: function [[ne]](hoc) */

/*s: function [[and]](hoc) */
void
and(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val != 0.0 && d2.val != 0.0);
    push(d1);
}
/*e: function [[and]](hoc) */

/*s: function [[or]](hoc) */
void
or(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = (double)(d1.val != 0.0 || d2.val != 0.0);
    push(d1);
}
/*e: function [[or]](hoc) */

/*s: function [[not]](hoc) */
void
not(void)
{
    Datum d;
    d = pop();
    d.val = (double)(d.val == 0.0);
    push(d);
}
/*e: function [[not]](hoc) */

/*s: function [[power]](hoc) */
void
power(void)
{
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    d1.val = Pow(d1.val, d2.val);
    push(d1);
}
/*e: function [[power]](hoc) */

/*s: function [[assign]](hoc) */
void
assign(void)
{
    Datum d1, d2;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != VAR && d1.sym->type != UNDEF)
        execerror("assignment to non-variable",
            d1.sym->name);
    d1.sym->u.val = d2.val;
    d1.sym->type = VAR;
    push(d2);
}
/*e: function [[assign]](hoc) */

/*s: function [[addeq]](hoc) */
void
addeq(void)
{
    Datum d1, d2;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != VAR && d1.sym->type != UNDEF)
        execerror("assignment to non-variable",
            d1.sym->name);
    d2.val = d1.sym->u.val += d2.val;
    d1.sym->type = VAR;
    push(d2);
}
/*e: function [[addeq]](hoc) */

/*s: function [[subeq]](hoc) */
void
subeq(void)
{
    Datum d1, d2;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != VAR && d1.sym->type != UNDEF)
        execerror("assignment to non-variable",
            d1.sym->name);
    d2.val = d1.sym->u.val -= d2.val;
    d1.sym->type = VAR;
    push(d2);
}
/*e: function [[subeq]](hoc) */

/*s: function [[muleq]](hoc) */
void
muleq(void)
{
    Datum d1, d2;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != VAR && d1.sym->type != UNDEF)
        execerror("assignment to non-variable",
            d1.sym->name);
    d2.val = d1.sym->u.val *= d2.val;
    d1.sym->type = VAR;
    push(d2);
}
/*e: function [[muleq]](hoc) */

/*s: function [[diveq]](hoc) */
void
diveq(void)
{
    Datum d1, d2;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != VAR && d1.sym->type != UNDEF)
        execerror("assignment to non-variable",
            d1.sym->name);
    d2.val = d1.sym->u.val /= d2.val;
    d1.sym->type = VAR;
    push(d2);
}
/*e: function [[diveq]](hoc) */

/*s: function [[modeq]](hoc) */
void
modeq(void)
{
    Datum d1, d2;
    long x;
    d1 = pop();
    d2 = pop();
    if (d1.sym->type != VAR && d1.sym->type != UNDEF)
        execerror("assignment to non-variable",
            d1.sym->name);
    /* d2.val = d1.sym->u.val %= d2.val; */
    x = d1.sym->u.val;
    x %= (long) d2.val;
    d2.val = d1.sym->u.val = x;
    d1.sym->type = VAR;
    push(d2);
}
/*e: function [[modeq]](hoc) */

/*s: function [[printtop]](hoc) */
void
printtop(void)  /* pop top value from stack, print it */
{
    Datum d;
    static Symbol *s;   /* last value computed */
    if (s == 0)
        s = install("_", VAR, 0.0);
    d = pop();
    print("%.12g\n", d.val);
    s->u.val = d.val;
}
/*e: function [[printtop]](hoc) */

/*s: function [[prexpr]](hoc) */
void
prexpr(void)    /* print numeric value */
{
    Datum d;
    d = pop();
    print("%.12g ", d.val);
}
/*e: function [[prexpr]](hoc) */

/*s: function [[prstr]](hoc) */
void
prstr(void)             /* print string value */ 
{
    print("%s", (char *) *pc++);
}
/*e: function [[prstr]](hoc) */

/*s: function [[varread]](hoc) */
void
varread(void)   /* read into variable */
{
    Datum d;
    extern Biobuf *bin;
    Symbol *var = (Symbol *) *pc++;
    int c;

  Again:
    do
        c = Bgetc(bin);
    while(c==' ' || c=='\t' || c=='\n');
    if(c == Beof){
  Iseof:
        if(moreinput())
            goto Again;
        d.val = var->u.val = 0.0;
        goto Return;
    }

    if(strchr("+-.0123456789", c) == 0)
        execerror("non-number read into", var->name);
    Bungetc(bin);
    if(Bgetd(bin, &var->u.val) == Beof)
        goto Iseof;
    else
        d.val = 1.0;
  Return:
    var->type = VAR;
    push(d);
}
/*e: function [[varread]](hoc) */

/*s: function [[code]](hoc) */
Inst*
code(Inst f)    /* install one instruction or operand */
{
    Inst *oprogp = progp;
    if (progp >= &prog[NPROG])
        execerror("program too big", (char *)0);
    *progp++ = f;
    return oprogp;
}
/*e: function [[code]](hoc) */

/*s: function [[execute]](hoc) */
void
execute(Inst* p)
{
    for (pc = p; *pc != STOP && !returning; )
        (*((++pc)[-1]))();
}
/*e: function [[execute]](hoc) */
/*e: hoc/code.c */
