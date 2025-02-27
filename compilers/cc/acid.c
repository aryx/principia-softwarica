/*s: cc/acid.c */
#include "cc.h"

/*s: global [[kwd]] */
static char *kwd[] =
{
    "$adt", "$aggr", "$append", "$builtin", "$complex", "$defn",
    "$delete", "$do", "$else", "$eval", "$head", "$if",
    "$local", "$loop", "$return", "$tail", "$then",
    "$union", "$whatis", "$while",
};
/*e: global [[kwd]] */

/*s: function [[amap]] */
char*
amap(char *s)
{
    int i, bot, top, new;

    bot = 0;
    top = bot + nelem(kwd) - 1;
    while(bot <= top){
        new = bot + (top - bot)/2;
        i = strcmp(kwd[new]+1, s);
        if(i == 0)
            return kwd[new];

        if(i < 0)
            bot = new + 1;
        else
            top = new - 1;
    }
    return s;
}
/*e: function [[amap]] */

/*s: function [[acidsue]] */
Sym*
acidsue(Type *t)
{
    int h;
    Sym *s;

    if(t != T)
    for(h=0; h<nelem(hash); h++)
        for(s = hash[h]; s != S; s = s->link)
            if(s->suetag && s->suetag->link == t)
                return s;
    return 0;
}
/*e: function [[acidsue]] */

/*s: function [[acidfun]] */
Sym*
acidfun(Type *t)
{
    int h;
    Sym *s;

    for(h=0; h<nelem(hash); h++)
        for(s = hash[h]; s != S; s = s->link)
            if(s->type == t)
                return s;
    return 0;
}
/*e: function [[acidfun]] */

/*s: global [[acidchar]] */
char	acidchar[NTYPE];
/*e: global [[acidchar]] */
/*s: global [[acidcinit]] */
Init	acidcinit[] =
{
    TCHAR,		'C',	0,
    TUCHAR,		'b',	0,
    TSHORT,		'd',	0,
    TUSHORT,	'u',	0,
    TLONG,		'D',	0,
    TULONG,		'U',	0,
    TVLONG,		'V',	0,
    TUVLONG,	'W',	0,
    TFLOAT,		'f',	0,
    TDOUBLE,	'F',	0,
    TARRAY,		'a',	0,
    TIND,		'X',	0,
    -1,		0,	0,
};
/*e: global [[acidcinit]] */

/*s: function [[acidinit]] */
static void
acidinit(void)
{
    Init *p;

    for(p=acidcinit; p->code >= 0; p++)
        acidchar[p->code] = p->value;

    acidchar[TINT] = acidchar[TLONG];
    acidchar[TUINT] = acidchar[TULONG];
    if(types[TINT]->width != types[TLONG]->width) {
        acidchar[TINT] = acidchar[TSHORT];
        acidchar[TUINT] = acidchar[TUSHORT];
        if(types[TINT]->width != types[TSHORT]->width)
            warn(Z, "acidmember int not long or short");
    }
    if(types[TIND]->width == types[TUVLONG]->width)
        acidchar[TIND] = 'Y';
    
}
/*e: function [[acidinit]] */

/*s: function [[acidmember]] */
void
acidmember(Type *t, long off, int flag)
{
    Sym *s, *s1;
    Type *l;
    static bool acidcharinit = false;

    if(acidcharinit == false) {
        acidinit();
        acidcharinit = true;
    }
    s = t->sym;
    switch(t->etype) {
    default:
        Bprint(&outbuf, "	T%d\n", t->etype);
        break;

    case TIND:
        if(s == S)
            break;
        if(flag) {
            for(l=t; l->etype==TIND; l=l->link)
                ;
            if(typesu[l->etype]) {
                s1 = acidsue(l->link);
                if(s1 != S) {
                    Bprint(&outbuf, "	'A' %s %ld %s;\n",
                        amap(s1->name),
                        t->offset+off, amap(s->name));
                    break;
                }
            }
        } else {
            Bprint(&outbuf,
                "\tprint(\"\t%s\t\", addr.%s\\X, \"\\n\");\n",
                amap(s->name), amap(s->name));
            break;
        }

    case TINT:
    case TUINT:
    case TCHAR:
    case TUCHAR:
    case TSHORT:
    case TUSHORT:
    case TLONG:
    case TULONG:
    case TVLONG:
    case TUVLONG:
    case TFLOAT:
    case TDOUBLE:
    case TARRAY:
        if(s == S)
            break;
        if(flag) {
            Bprint(&outbuf, "	'%c' %ld %s;\n",
            acidchar[t->etype], t->offset+off, amap(s->name));
        } else {
            Bprint(&outbuf, "\tprint(\"\t%s\t\", addr.%s, \"\\n\");\n",
                amap(s->name), amap(s->name));
        }
        break;

    case TSTRUCT:
    case TUNION:
        s1 = acidsue(t->link);
        if(s1 == S)
            break;
        if(flag) {
            if(s == S) {
                Bprint(&outbuf, "	{\n");
                for(l = t->link; l != T; l = l->down)
                    acidmember(l, t->offset+off, flag);
                Bprint(&outbuf, "	};\n");
            } else {
                Bprint(&outbuf, "	%s %ld %s;\n",
                    amap(s1->name),
                    t->offset+off, amap(s->name));
            }
        } else {
            if(s != S) {
                Bprint(&outbuf, "\tprint(\"%s %s {\\n\");\n",
                    amap(s1->name), amap(s->name));
                Bprint(&outbuf, "\t%s(addr.%s);\n",
                    amap(s1->name), amap(s->name));
                Bprint(&outbuf, "\tprint(\"}\\n\");\n");
            } else {
                Bprint(&outbuf, "\tprint(\"%s {\\n\");\n",
                    amap(s1->name));
                Bprint(&outbuf, "\t\t%s(addr+%ld);\n",
                    amap(s1->name), t->offset+off);
                Bprint(&outbuf, "\tprint(\"}\\n\");\n");
            }
        }
        break;
    }
}
/*e: function [[acidmember]] */

/*s: function [[acidtype]] */
void
acidtype(Type *t)
{
    Sym *s;
    Type *l;
    Io *i;
    int n;
    char *an;

    /*s: [[acidtype()]] return if no -a */
    if(!debug['a'])
        return;
    /*e: [[acidtype()]] return if no -a */
    /*s: [[acidtype()]] if -aa */
    if(debug['a'] > 1) {
        n = 0;
        for(i=iostack; i; i=i->link)
            n++;
        if(n > 1)
            return;
    }
    /*e: [[acidtype()]] if -aa */

    s = acidsue(t->link);
    if(s == S)
        return;
    switch(t->etype) {
    case TUNION:
    case TSTRUCT:
        if(debug['s'])
            goto asmstr;
        an = amap(s->name);
        Bprint(&outbuf, "sizeof%s = %ld;\n", an, t->width);
        Bprint(&outbuf, "aggr %s\n{\n", an);
        for(l = t->link; l != T; l = l->down)
            acidmember(l, 0, 1);
        Bprint(&outbuf, "};\n\n");

        Bprint(&outbuf, "defn\n%s(addr) {\n\tcomplex %s addr;\n", an, an);
        for(l = t->link; l != T; l = l->down)
            acidmember(l, 0, 0);
        Bprint(&outbuf, "};\n\n");
        break;

    asmstr:
        if(s == S)
            break;
        for(l = t->link; l != T; l = l->down)
            if(l->sym != S)
                Bprint(&outbuf, "#define\t%s.%s\t%ld\n",
                    s->name,
                    l->sym->name,
                    l->offset);
        break;

    default:
        Bprint(&outbuf, "T%d\n", t->etype);
        return;

    }
}
/*e: function [[acidtype]] */

/*s: function [[acidvar]] */
void
acidvar(Sym *s)
{
    int n;
    Io *i;
    Type *t;
    Sym *s1, *s2;

    /*s: [[acidvar()]] return if no -a or -s */
    if(!debug['a'] || debug['s'])
        return;
    /*e: [[acidvar()]] return if no -a or -s */
    /*s: [[acidvar()]] if -aa */
    if(debug['a'] > 1) {
        n = 0;
        for(i=iostack; i; i=i->link)
            n++;
        if(n > 1)
            return;
    }
    /*e: [[acidvar()]] if -aa */

    t = s->type;
    while(t && t->etype == TIND)
        t = t->link;
    if(t == T)
        return;
    if(t->etype == TENUM) {
        Bprint(&outbuf, "%s = ", amap(s->name));
        if(!typefd[t->etype])
            Bprint(&outbuf, "%lld;\n", s->vconst);
        else
            Bprint(&outbuf, "%f\n;", s->fconst);
        return;
    }
    if(!typesu[t->etype])
        return;
    s1 = acidsue(t->link);
    if(s1 == S)
        return;
    switch(s->class) {
    case CAUTO:
    case CPARAM:
        s2 = acidfun(thisfntype);
        if(s2)
            Bprint(&outbuf, "complex %s %s:%s;\n",
                amap(s1->name), amap(s2->name), amap(s->name));
        break;
    
    case CEXTERN:
    case CGLOBL:
    case CSTATIC: case CLOCAL:
        Bprint(&outbuf, "complex %s %s;\n",
            amap(s1->name), amap(s->name));
        break;
    }
}
/*e: function [[acidvar]] */
/*e: cc/acid.c */
