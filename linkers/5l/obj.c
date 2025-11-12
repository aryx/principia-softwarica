/*s: linkers/5l/obj.c */
#include	"l.h"
#include	<ar.h>

// Reading object files.

/*s: global noname linker */
char	*noname		= "<none>";
/*e: global noname linker */
/*s: global symname linker */
char	symname[]	= SYMDEF;
/*e: global symname linker */

/*s: global [[version]] */
int	version = 0;
/*e: global [[version]] */

/*s: global [[literal]](arm) */
char	literal[32];
/*e: global [[literal]](arm) */

/*s: function [[isobjfile]] */
int
isobjfile(char *f)
{
    int n, v;
    Biobuf *b;
    char buf1[5], buf2[SARMAG];

    b = Bopen(f, OREAD);
    if(b == nil)
        return 0;
    n = Bread(b, buf1, 5);
    if(n == 5 && (buf1[2] == 1 && buf1[3] == '<' || buf1[3] == 1 && buf1[4] == '<'))
        v = 1;	/* good enough for our purposes */
    else{
        Bseek(b, 0, 0);
        n = Bread(b, buf2, SARMAG);
        v = n == SARMAG && strncmp(buf2, ARMAG, SARMAG) == 0;
    }
    Bterm(b);
    return v;
}
/*e: function [[isobjfile]] */


/*s: function [[inopd]](arm) */
/// main -> objfile -> ldobj -> <>
static int
inopd(byte *p, Adr *a, Sym *h[])
{
    int size; // returned
    int symidx;
    /*s: [[inopd()]] other locals */
    Sym *s;
    // <enum<Sym_kind>>
    int t;
    int l;
    Auto *u;
    /*e: [[inopd()]] other locals */

    a->type = p[0];
    a->reg = p[1];
    /*s: [[inopd()]] sanity check register range */
    if(a->reg < 0 || a->reg > NREG) {
        print("register out of range %d\n", a->reg);
        p[0] = ALAST+1;
        return 0;	/*  force real diagnostic */
    }
    /*e: [[inopd()]] sanity check register range */
    symidx = p[2];
    /*s: [[inopd()]] sanity check symbol range */
    if(symidx < 0 || symidx > NSYM){
        print("sym out of range: %d\n", symidx);
        p[0] = ALAST+1;
        return 0;
    }
    /*e: [[inopd()]] sanity check symbol range */
    a->sym = h[symidx];
    a->symkind = p[3];
    /*s: [[inopd()]] sanity check [[D_CONST]] */
    if(a->type == D_CONST && a->symkind != N_NONE) {
        a->type = D_ADDR;
        //print("missing D_CONST -> D_ADDR\n");
        //p[0] = ALAST+1;
        //return 0;	/*  force real diagnostic */
    }
    //if(a->type == D_ADDR && a->symkind == N_NONE) {
    //    print("wrong D_CONST -> D_ADDR\n");
    //    p[0] = ALAST+1;
    //    return 0;	/*  force real diagnostic */
    //}
    /*e: [[inopd()]] sanity check [[D_CONST]] */

    size = 4;

    switch(a->type) {
    /*s: [[inopd()]] cases */
    // 0 byte
    case D_NONE:
    case D_REG:
    case D_PSR:
        break;

    // 1 byte
    case D_REGREG:
        a->offset = p[4];
        size++;
        break;

    // 4 bytes
    case D_CONST:
    // ??? D_ADDR is 4 bytes?? what about symbol?
    case D_ADDR:
    case D_SHIFT:
    case D_OREG:
    case D_BRANCH:
        a->offset = p[4] | (p[5]<<8) | (p[6]<<16) | (p[7]<<24);
        size += 4;
        break;

    // 8 bytes (NSNAME)
    case D_SCONST:
        a->sval = malloc(NSNAME);
        memmove(a->sval, p+4, NSNAME);
        size += NSNAME;
        break;
    /*x: [[inopd()]] cases */
    case D_FREG:
    case D_FPCR:
        break;
    /*x: [[inopd()]] cases */
    case D_FCONST:
        a->ieee = malloc(sizeof(Ieee));

        a->ieee->l = p[4] | (p[5]<<8) | (p[6]<<16) | (p[7]<<24);
        a->ieee->h = p[8] | (p[9]<<8) | (p[10]<<16) | (p[11]<<24);
        size += 8;
        break;
    /*e: [[inopd()]] cases */
    default:
        print("unknown type %d\n", a->type);
        p[0] = ALAST+1;
        return 0;	/*  force real diagnostic */

    }
    /*s: [[inopd()]] adjust curauto for [[N_LOCAL]] or [[N_PARAM]] symkind */
    s = a->sym;
    t = a->symkind;
    l = a->offset;

    // a parameter or local with a symbol, e.g., p+4(FP)
    if(s != S && (t == N_LOCAL || t == N_PARAM)) {
  
       /*s: [[inopd()]] return if stack variable already present in curauto */
       for(u=curauto; u; u=u->link)
           if(u->asym == s)
            if(u->type == t) {
               if(u->aoffset > l)
                   u->aoffset = l; // diag()? inconsistent offset?
               return size;
           }
       /*e: [[inopd()]] return if stack variable already present in curauto */
        // else
    
        u = malloc(sizeof(Auto));
        u->asym = s;
        u->type = t;
        u->aoffset = l;
    
        //add_list(u, curauto)
        u->link = curauto;
        curauto = u;
    }
    /*e: [[inopd()]] adjust curauto for [[N_LOCAL]] or [[N_PARAM]] symkind */

    return size;
}
/*e: function [[inopd]](arm) */



/*s: function [[collapsefrog]] */
static void
collapsefrog(Sym *s)
{
    int i;

    /*
     * bad encoding of path components only allows
     * MAXHIST components. if there is an overflow,
     * first try to collapse xxx/..
     */
    for(i=1; i<histfrogp; i++)
        if(strcmp(histfrog[i]->name+1, "..") == 0) {
            memmove(histfrog+i-1, histfrog+i+1,
                (histfrogp-i-1)*sizeof(histfrog[0]));
            histfrogp--;
            goto out;
        }

    /*
     * next try to collapse .
     */
    for(i=0; i<histfrogp; i++)
        if(strcmp(histfrog[i]->name+1, ".") == 0) {
            memmove(histfrog+i, histfrog+i+1,
                (histfrogp-i-1)*sizeof(histfrog[0]));
            goto out;
        }

    /*
     * last chance, just truncate from front
     */
    memmove(histfrog+0, histfrog+1,
        (histfrogp-1)*sizeof(histfrog[0]));

out:
    histfrog[histfrogp-1] = s;
}
/*e: function [[collapsefrog]] */

/*s: function [[nopout]] */
static void
nopout(Prog *p)
{
    p->as = ANOP;
    p->from.type = D_NONE;
    p->to.type = D_NONE;
}
/*e: function [[nopout]] */


/*s: function [[ldobj]](arm) */
/// main -> objfile -> <>
void
ldobj(fdt f, long c, char *pn)
{
    /*s: [[ldobj()]] locals(arm) */
    long ipc;
    /*x: [[ldobj()]] locals(arm) */
    Prog *p;
    // enum<Opcode>
    short o;
    /*x: [[ldobj()]] locals(arm) */
    // array<byte> (slice of buf.ibuf)
    byte *bloc;
    // ref<byte> (end pointer in buf.ibuf)
    byte *bsize;
    // remaining bytes, bsize - bloc
    int r;
    /*x: [[ldobj()]] locals(arm) */
    // array<option<ref<Sym>>>
    Sym *h[NSYM];
    /*x: [[ldobj()]] locals(arm) */
    // enum<Sym_kind>
    int k;
    int symidx;
    int v;
    // ref<byte> (in Buf.ibuf)
    byte *stop;
    /*x: [[ldobj()]] locals(arm) */
    Sym *s;
    /*x: [[ldobj()]] locals(arm) */
    ulong sig;
    /*x: [[ldobj()]] locals(arm) */
    // growing_array<option<string>>  (grown for every 16 elements)
    static char **filen;
    // index of next free entry in filen
    static int files = 0;
    /*x: [[ldobj()]] locals(arm) */
    char **nfilen; // new filen
    /*x: [[ldobj()]] locals(arm) */
    bool skip;
    /*x: [[ldobj()]] locals(arm) */
    Prog *t;
    /*e: [[ldobj()]] locals(arm) */

    /*s: [[ldobj()]] remember set of object filenames */
    /*s: [[ldobj()]] grow filen if not enough space */
    if((files&15) == 0){
        nfilen = malloc((files+16)*sizeof(char*));
        memmove(nfilen, filen, files*sizeof(char*));
        free(filen);
        filen = nfilen;
    }
    /*e: [[ldobj()]] grow filen if not enough space */
    filen[files++] = strdup(pn);
    /*e: [[ldobj()]] remember set of object filenames */
    /*s: [[ldobj()]] bloc and bsize init */
    bloc = buf.ibuf;
    bsize = buf.ibuf;
    /*e: [[ldobj()]] bloc and bsize init */

// can come from AEND
newloop:
    // new object file
    ipc = pc;
    /*s: [[ldobj()]] after newloop when new object file, more initializations */
    memset(h, 0, sizeof(h));
    /*x: [[ldobj()]] after newloop when new object file, more initializations */
    version++;
    /*x: [[ldobj()]] after newloop when new object file, more initializations */
    histfrogp = 0;
    /*x: [[ldobj()]] after newloop when new object file, more initializations */
    skip = false;
    /*e: [[ldobj()]] after newloop when new object file, more initializations */

loop:
    if(c <= 0)
        goto eof;

    /*s: [[ldobj()]] read if needed in loop:, adjust bloc and bsize */
    r = bsize - bloc;
    if(r < 100 && r < c) {		/* enough for largest instruction */
        bsize = readsome(f, buf.ibuf, bloc, bsize, c);
        if(bsize == nil)
            goto eof;
        bloc = buf.ibuf; // readsome() does some memmove()
        goto loop;
    }
    /*e: [[ldobj()]] read if needed in loop:, adjust bloc and bsize */

    o = bloc[0];		/* as */
    /*s: [[ldobj()]] sanity check opcode in range(arm) */
    if(o <= AXXX || o >= ALAST) {
        diag("%s: line %ld: opcode out of range %d", pn, pc-ipc, o);
        print("	probably not a .5 file\n");
        errorexit();
    }
    /*e: [[ldobj()]] sanity check opcode in range(arm) */

    // dispatch opcode part one
    /*s: [[ldobj()]] if ANAME or ASIGNAME(arm) */
    if(o == ANAME || o == ASIGNAME) {
        /*s: [[ldobj()]] if SIGNAME adjust sig */
        sig = 0;
        if(o == ASIGNAME){
            sig = bloc[1] | (bloc[2]<<8) | (bloc[3]<<16) | (bloc[4]<<24);
            bloc += 4;
            c -= 4;
        }
        /*e: [[ldobj()]] if SIGNAME adjust sig */

        stop = memchr(&bloc[3], '\0', bsize-&bloc[3]);
        /*s: [[ldobj()]] if stop is nil refill buffer and retry */
        if(stop == nil){
            bsize = readsome(f, buf.ibuf, bloc, bsize, c);
            if(bsize == nil)
                goto eof;
            bloc = buf.ibuf;
            stop = memchr(&bloc[3], '\0', bsize-&bloc[3]);
            if(stop == nil){
                fprint(2, "%s: name too long\n", pn);
                errorexit();
            }
        }
        /*e: [[ldobj()]] if stop is nil refill buffer and retry */

        k = bloc[1];	/* type */
        symidx = bloc[2];	/* sym */

        bloc += 3;
        c -= 3;

        v = 0; // global version by default
        /*s: [[ldobj()]] when ANAME opcode, if private symbol adjust version */
        if(k == N_INTERN)
            v = version;
        /*e: [[ldobj()]] when ANAME opcode, if private symbol adjust version */

        // this will possibly create new symbols
        s = lookup((char*)bloc, v);

        c -= &stop[1] - bloc;
        bloc = stop + 1;

        /*s: [[ldobj()]] if sig not zero */
        if(sig != 0){
            /*s: [[ldobj()]] signature compatibility check */
            if(s->sig != 0 && s->sig != sig)
                diag("incompatible type signatures %lux(%s) and %lux(%s) for %s", 
                     s->sig, filen[s->file], 
                     sig, pn, 
                     s->name);
            /*e: [[ldobj()]] signature compatibility check */
            s->sig = sig;
            /*s: [[ldobj()]] remember file introducing the symbol */
            s->file = files-1;
            /*e: [[ldobj()]] remember file introducing the symbol */
        }
        /*e: [[ldobj()]] if sig not zero */
        /*s: [[ldobj()]] when ANAME, debug */
        if(debug['W'])
            print("	ANAME	%s\n", s->name);
        /*e: [[ldobj()]] when ANAME, debug */

        h[symidx] = s;

        if((k == N_EXTERN || k == N_INTERN) && s->type == SNONE)
            s->type = SXREF;

        /*s: [[ldobj()]] when ANAME opcode, if [[N_FILE]] */
        if(k == N_FILE) {
            if(s->type != SFILE) {
                s->type = SFILE;
                histgen++;
                s->value = histgen;
            }
            /*s: [[ldobj()]] when ANAME opcode, if [[N_FILE]], update histfrogp */
            if(histfrogp < MAXHIST) {
                histfrog[histfrogp] = s;
                histfrogp++;
            } 
            /*e: [[ldobj()]] when ANAME opcode, if [[N_FILE]], update histfrogp */
            /*s: [[ldobj()]] when ANAME opcode, if [[N_FILE]], if no more space in histfrog */
            else
                    collapsefrog(s);
            /*e: [[ldobj()]] when ANAME opcode, if [[N_FILE]], if no more space in histfrog */
        }
        /*e: [[ldobj()]] when ANAME opcode, if [[N_FILE]] */
        goto loop;
    }
    /*e: [[ldobj()]] if ANAME or ASIGNAME(arm) */
    // else

    p = malloc(sizeof(Prog));
    p->as = o;
    /*s: [[ldobj()]] read one instruction in p */
    // mostly opposite of outcode() in 5a
    // p->as = bloc[0] has been done already above so continue from bloc[1]
    p->scond = bloc[1];
    p->reg   = bloc[2];
    p->line  = bloc[3] | (bloc[4]<<8) | (bloc[5]<<16) | (bloc[6]<<24);
    r = 7;
    r += inopd(bloc+r, &p->from, h);
    r += inopd(bloc+r, &p->to, h);

    bloc += r;
    c -= r;
    /*e: [[ldobj()]] read one instruction in p */
    p->link = P;
    p->cond = P;

    /*s: [[ldobj()]] sanity check p */
    if(p->reg > NREG)
        diag("register out of range %d", p->reg);
    /*e: [[ldobj()]] sanity check p */
    /*s: [[ldobj()]] debug */
    if(debug['W'])
        print("%P\n", p);
    /*e: [[ldobj()]] debug */

    // dispatch opcode part two
    switch(o) {
    /*s: [[ldobj()]] switch opcode cases(arm) */
    default:
    casedef:
        /*s: [[ldobj()]] in switch opcode default case, if skip */
        if(skip)
            nopout(p);
        /*e: [[ldobj()]] in switch opcode default case, if skip */

        // relocation
        if(p->to.type == D_BRANCH)
            p->to.offset += ipc;

        //add_queue(firstp, lastp, p)
        lastp->link = p;
        lastp = p;

        p->pc = pc;
        pc++;
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case ATEXT:
        /*s: [[ldobj()]] case ATEXT, if curtext not null adjustments for curauto */
        if(curtext != P) {
            /*s: [[ldobj()]] case ATEXT, curauto adjustments with curhist */
            histtoauto();
            /*e: [[ldobj()]] case ATEXT, curauto adjustments with curhist */
            curtext->to.autom = curauto;
            curauto = nil;
        }
        /*e: [[ldobj()]] case ATEXT, if curtext not null adjustments for curauto */
        curtext = p;
        /*s: [[ldobj()]] in switch opcode ATEXT case, reset skip */
        skip = false; // needed?
        /*e: [[ldobj()]] in switch opcode ATEXT case, reset skip */
        /*s: [[ldobj()]] in switch opcode ATEXT case, set autosize */
        p->to.offset = rnd(p->to.offset, 4);
        autosize = p->to.offset;
        autosize += 4;
        /*e: [[ldobj()]] in switch opcode ATEXT case, set autosize */

        s = p->from.sym;
        /*s: [[ldobj()]] sanity check for ATEXT symbol s */
        if(s == S) {
            diag("TEXT must have a name\n%P", p);
            errorexit();
        }
        if(!(s->type == SNONE || s->type == SXREF)) {
            /*s: [[ldobj()]] case ATEXT and section not SNONE or SXREF, if DUPOK */
            if(p->reg & DUPOK) {
                skip = true;
                goto casedef;
            }
            /*e: [[ldobj()]] case ATEXT and section not SNONE or SXREF, if DUPOK */
            diag("redefinition: %s\n%P", s->name, p);
        }
        /*e: [[ldobj()]] sanity check for ATEXT symbol s */
        s->type = STEXT;
        s->value = pc;

        // like in default case
        //add_queue(firstp, lastp, p)
        lastp->link = p;
        lastp = p;

        p->pc = pc;
        pc++;

        /*s: [[ldobj()]] in switch opcode ATEXT case, populate textp */
        //add_queue(textp, etextp, p)
        if(textp == P) {
            textp = p;
            etextp = p;
        } else {
            etextp->cond = p;
            etextp = p;
        }
        /*e: [[ldobj()]] in switch opcode ATEXT case, populate textp */
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AGLOBL:
        s = p->from.sym;
        /*s: [[ldobj()]] sanity check for AGLOBL symbol s */
        if(s == S) {
            diag("GLOBL must have a name\n%P", p);
            errorexit();
        }
        if(!(s->type == SNONE || s->type == SXREF))
            diag("redefinition: %s\n%P", s->name, p);
        /*e: [[ldobj()]] sanity check for AGLOBL symbol s */
        s->type = SBSS; // for now; will be set maybe to SDATA in dodata()
        s->value = (p->to.offset > 0) ? p->to.offset : 0;
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case ADATA:
        /*s: [[ldobj()]] sanity check for ADATA symbol s */
        if(p->from.sym == S) {
            diag("DATA without a sym\n%P", p);
            break;
        }
        /*e: [[ldobj()]] sanity check for ADATA symbol s */

        //add_list(datap, p)
        p->link = datap;
        datap = p;

        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AEND:
        /*s: [[ldobj()]] case AEND, curauto adjustments with curhist */
        histtoauto();
        /*e: [[ldobj()]] case AEND, curauto adjustments with curhist */
        /*s: [[ldobj()]] case AEND, curauto adjustments */
        if(curtext != P)
            curtext->to.autom = curauto;
        curauto = nil;
        /*e: [[ldobj()]] case AEND, curauto adjustments */
        curtext = P;

        if(c)
            goto newloop;
        return;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AGOK:
        diag("unknown opcode\n%P", p);
        p->pc = pc;
        pc++;
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AHISTORY:
        /*s: [[ldobj()]] in AHISTORY case, if pragma lib */
        if(p->to.offset == -1) {
            addlib(pn);
            histfrogp = 0;
            goto loop;
        }
        /*e: [[ldobj()]] in AHISTORY case, if pragma lib */
        // else

        // the global line
        addhist(p->line, N_FILE);		/* 'z' */
        // the local line (if needed for #line)
        if(p->to.offset)
            addhist(p->to.offset, N_LINE);	/* 'Z' */
        /*s: [[ldobj()]] in AHISTORY case, end of case, reset histfrogp */
        histfrogp = 0;
        /*e: [[ldobj()]] in AHISTORY case, end of case, reset histfrogp */
        goto loop;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case ASUB:
        if(p->from.type == D_CONST)
          if(p->from.offset < 0) {
            p->from.offset = -p->from.offset;
            p->as = AADD;
        }
        goto casedef;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AADD:
        if(p->from.type == D_CONST)
          if(p->from.offset < 0) {
            p->from.offset = -p->from.offset;
            p->as = ASUB;
        }
        goto casedef;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AMOVDF:
        if(!vfp || p->from.type != D_FCONST)
            goto casedef;
        p->as = AMOVF;
        /* fall through */
    case AMOVF:
        if(skip)
            goto casedef;

        if(p->from.type == D_FCONST && chipfloat(p->from.ieee) < 0) {
            /* size sb 9 max */
            sprint(literal, "$%lux", ieeedtof(p->from.ieee));
            s = lookup(literal, 0);
            if(s->type == 0) {
                s->type = SBSS;
                s->value = 4;
                t = prg();
                t->as = ADATA;
                t->line = p->line;
                t->from.type = D_OREG;
                t->from.sym = s;
                t->from.symkind = N_EXTERN;
                t->reg = 4;
                t->to = p->from;
                t->link = datap;
                datap = t;
            }
            p->from.type = D_OREG;
            p->from.sym = s;
            p->from.symkind = N_EXTERN;
            p->from.offset = 0;
        }
        goto casedef;

    case AMOVD:
        if(skip)
            goto casedef;

        if(p->from.type == D_FCONST && chipfloat(p->from.ieee) < 0) {
            /* size sb 18 max */
            sprint(literal, "$%lux.%lux",
                p->from.ieee->l, p->from.ieee->h);
            s = lookup(literal, 0);
            if(s->type == 0) {
                s->type = SBSS;
                s->value = 8;
                t = prg();
                t->as = ADATA;
                t->line = p->line;
                t->from.type = D_OREG;
                t->from.sym = s;
                t->from.symkind = N_EXTERN;
                t->reg = 8;
                t->to = p->from;
                t->link = datap;
                datap = t;
            }
            p->from.type = D_OREG;
            p->from.sym = s;
            p->from.symkind = N_EXTERN;
            p->from.offset = 0;
        }
        goto casedef;
    /*e: [[ldobj()]] switch opcode cases(arm) */
    }
    goto loop;

eof:
    diag("truncated object file: %s", pn);
}
/*e: function [[ldobj]](arm) */

/*s: function [[objfile]] */
/// main | loadlib  -> <>
void
objfile(char *file)
{
    fdt f;
    long len;
    char magbuf[SARMAG]; // magic buffer
    /*s: [[objfile()]] other locals */
    struct ar_hdr arhdr;
    long off, esym, cnt;
    Sym *s;
    char pname[LIBNAMELEN];
    char name[LIBNAMELEN];
    char *e, *start, *stop;
    bool work;
    int pass = 1;
    /*e: [[objfile()]] other locals */

    DBG("%5.2f objfile: %s\n", cputime(), file);

    /*s: [[objfile()]] adjust file if -lxxx filename */
    if(file[0] == '-' && file[1] == 'l') {
        snprint(pname, sizeof(pname), "lib%s.a", file+2);
        e = findlib(pname);
        if(e == nil) {
            diag("cannot find library: %s", file);
            errorexit();
        }
        snprint(name, sizeof(name), "%s/%s", e, pname);
        file = name;
    }
    /*e: [[objfile()]] adjust file if -lxxx filename */

    f = open(file, 0);
    /*s: [[objfile()]] sanity check f */
    if(f < 0) {
        diag("cannot open %s: %r", file);
        errorexit();
    }
    /*e: [[objfile()]] sanity check f */

    len = read(f, magbuf, SARMAG);

    // is it a regular object? (not a library)
    if(len != SARMAG || strncmp(magbuf, ARMAG, SARMAG)){
        /* load it as a regular file */
        len = seek(f, 0L, SEEK__END); // len = filesize(f);
        seek(f, 0L, SEEK__START);

        // the important call!
        ldobj(f, len, file);

        close(f);
        return;
    }
    // else
    /*s: [[objfile()]] when file is a library */
    DBG("%5.2f ldlib: %s\n", cputime(), file);

    len = read(f, &arhdr, SAR_HDR);

    /*s: [[objfile()]] sanity check library header size and content */
    if(len != SAR_HDR) {
        diag("%s: short read on archive file symbol header", file);
        goto out;
    }
    if(strncmp(arhdr.name, symname, strlen(symname))) {
        diag("%s: first entry not symbol header", file);
        goto out;
    }
    /*e: [[objfile()]] sanity check library header size and content */

    esym = SARMAG + SAR_HDR + atolwhex(arhdr.size);
    off = SARMAG + SAR_HDR;

    /*
     * just bang the whole symbol file into memory
     */
    seek(f, off, 0);
    cnt = esym - off;
    start = malloc(cnt + 10);
    cnt = read(f, start, cnt);
    if(cnt <= 0){
        close(f);
        return;
    }
    stop = &start[cnt];
    memset(stop, '\0', 10);

    work = true;
    while(work) {

        DBG("%5.2f library pass%d: %s\n", cputime(), pass, file);
        pass++;
        work = false;
        for(e = start; e < stop; e = strchr(e+5, 0) + 1) {

            s = lookup(e+5, 0);
            // loading only the object files containing symbols we are looking for
            if(s->type == SXREF || 
               (s->type == SNONE && strcmp(s->name, "main") == 0)) {
                sprint(pname, "%s(%s)", file, s->name);
                DBG("%5.2f library: %s\n", cputime(), pname);
            
                len = e[1] & 0xff;
                len |= (e[2] & 0xff) << 8;
                len |= (e[3] & 0xff) << 16;
                len |= (e[4] & 0xff) << 24;
                // >> >> >> >>
            
                seek(f, len, SEEK__START);
                len = read(f, &arhdr, SAR_HDR);
                /*s: [[objfile()]] sanity check entry header */
                if(len != SAR_HDR)
                    goto bad;
                if(strncmp(arhdr.fmag, ARFMAG, sizeof(arhdr.fmag)))
                    goto bad;
                /*e: [[objfile()]] sanity check entry header */
                len = atolwhex(arhdr.size);

                // loading the object file containing the symbol
                ldobj(f, len, pname);
            
                if(s->type == SXREF) {
                    diag("%s: failed to load: %s", file, s->name);
                    errorexit();
                }
                work = true; // maybe some new SXREF has been found in ldobj()
               /*s: [[objfile()]] an SXREF was found hook */
               xrefresolv = true;
               /*e: [[objfile()]] an SXREF was found hook */
            }
        }
    }
    return;

    bad:
        diag("%s: bad or out of date archive", file);
    out:
        close(f);
    /*e: [[objfile()]] when file is a library */
}
/*e: function [[objfile]] */

/*e: linkers/5l/obj.c */
