/*s: cc/cc.y */
%{
#include "cc.h"
%}
/*s: union yacc */
%union  {
    vlong   vval;
    double  dval;
    struct
    {
        char*   s;
        long    l;
    } sval;

    Sym*    sym;

   /*s: [[union yacc]] other fields */
   long    lval;
   /*x: [[union yacc]] other fields */
   Node*   node;
   /*x: [[union yacc]] other fields */
   Type*   type;
   /*x: [[union yacc]] other fields */
   struct
   {
       Type*   t;
       // option<enum<storage_class> >, None = CXXX
       byte   c;
   } tycl;
   /*x: [[union yacc]] other fields */
   struct
   {
       Type*   t1;
       Type*   t2;
       Type*   t3;
       uchar   c;
   } tyty;
   /*e: [[union yacc]] other fields */
}
/*e: union yacc */
/*s: token declarations */
%token  <sym>   LNAME LTYPE
%token  <vval>  LCONST LLCONST LUCONST LULCONST LVLCONST LUVLCONST
%token  <dval>  LFCONST LDCONST
%token  <sval>  LSTRING LLSTRING
/*x: token declarations */
%token  LAUTO LBREAK LCASE LCHAR LCONTINUE LDEFAULT LDO
%token  LDOUBLE LELSE LEXTERN LFLOAT LFOR LGOTO
%token  LIF LINT LLONG LREGISTER LRETURN LSHORT LSIZEOF LUSED
%token  LSTATIC LSTRUCT LSWITCH LTYPEDEF LTYPESTR LUNION LUNSIGNED
%token  LWHILE LVOID LENUM LSIGNED LCONSTNT LVOLATILE LSET LSIGNOF
%token  LRESTRICT LINLINE
/*e: token declarations */
/*s: priority and associativity declarations */
%left   ';'
%left   ','
%right  '=' LPE LME LMLE LDVE LMDE LRSHE LLSHE LANDE LXORE LORE
%right  '?' ':'
%left   LOROR
%left   LANDAND
%left   '|'
%left   '^'
%left   '&'
%left   LEQ LNE
%left   '<' '>' LLE LGE
%left   LLSH LRSH
%left   '+' '-'
%left   '*' '/' '%'
%right  LMM LPP LMG '.' '[' '('
/*e: priority and associativity declarations */
/*s: type declarations */
%type   <node>  name tag 
%type   <node>  stmnt   block slist ulstmnt labels label forexpr
%type   <node>  expr    cexpr lexpr  xuexpr uexpr pexpr  elist  string lstring
%type   <node>  adecl   qual init ilist  adlist qlist  arglist
%type   <node>  xdecor xdecor2 edecor  abdecor abdecor1 abdecor2 abdecor3
%type   <node>  zexpr zelist zcexpr zarglist
/*x: type declarations */
%type   <lval>  tname cname gname
/*x: type declarations */
%type   <lval>   gcname   gctname
/*x: type declarations */
%type   <type> tlist
/*x: type declarations */
%type   <tycl>  types
/*x: type declarations */
%type   <lval>  gctnlist gcnlist zgnlist
/*x: type declarations */
%type   <type>  complex
/*x: type declarations */
%type   <sym>   ltag
/*x: type declarations */
%type   <type> sbody
/*e: type declarations */
%%
/*s: grammar */
prog:
  /* empty */
|   prog xdecl

/*s: declarator rules */
/*s: external declarator rules */
/*
 * external declarator
 */
xdecl:
    zctlist ';'          { dodecl(xdecl, lastclass, lasttype, Z); }
|   zctlist xdlist ';'
/*s: xdecl function definition case */
|   zctlist xdecor
    {
        lastdcl = T;
        firstarg = S;
        dodecl(xdecl, lastclass, lasttype, $2);
        if(lastdcl == T || lastdcl->etype != TFUNC) {
            diag($2, "not a function");
            lastdcl = types[TFUNC];
        }
        thisfn = lastdcl;
        markdcl();
        firstdcl = dclstack;
        argmark($2, 0);
    }
    pdecl
    {
        argmark($2, 1);
    }
    block
    {
        Node *n;

        n = revertdcl();
        if(n)
            $6 = new(OLIST, n, $6);

        if(!debug['a'] && !debug['Z'])
            codgen($6, $2); // !!!!!!!!!!!!!!!!!!!!!
    }
/*e: xdecl function definition case */
/*x: external declarator rules */
xdlist:
    xdecor  { dodecl(xdecl, lastclass, lasttype, $1); }
|   xdecor
    {
        $1 = dodecl(xdecl, lastclass, lasttype, $1);
    }
    '=' init
    {
        doinit($1->sym, $1->type, 0L, $4);
    }
|   xdlist ',' xdlist
/*x: external declarator rules */
xdecor:
    xdecor2
|   '*' zgnlist xdecor
    {
        $$ = new(OIND, $3, Z);
        $$->garb = simpleg($2);
    }
/*x: external declarator rules */
xdecor2:
    tag
|   '(' xdecor ')'             { $$ = $2;   }
|   xdecor2 '(' zarglist ')'   { $$ = new(OFUNC, $1, $3); }
|   xdecor2 '[' zexpr ']'      { $$ = new(OARRAY, $1, $3); }
/*e: external declarator rules */
/*s: automatic declarator rules */
/*
 * automatic declarator
 */
adecl:
    ctlist ';'        { $$ = dodecl(adecl, lastclass, lasttype, Z); }
|   ctlist adlist ';' { $$ = $2; }

adlist:
    xdecor
    {
        dodecl(adecl, lastclass, lasttype, $1);
        $$ = Z;
    }
|   xdecor
    {
        $1 = dodecl(adecl, lastclass, lasttype, $1);
    }
    '=' init
    {
        long w;

        w = $1->sym->type->width;
        $$ = doinit($1->sym, $1->type, 0L, $4);
        $$ = contig($1->sym, $$, w);
    }
|   adlist ',' adlist
    {
        $$ = $1;
        if($3 != Z) {
            $$ = $3;
            if($1 != Z)
                $$ = new(OLIST, $1, $3);
        }
    }
/*e: automatic declarator rules */
/*s: parameter declarator rules */
zarglist:
  /* empty */   { $$ = Z; }
|   arglist     { $$ = invert($1); }


arglist:
    name
|   tlist abdecor
    {
        $$ = new(OPROTO, $2, Z);
        $$->type = $1;
    }
|   tlist xdecor
    {
        $$ = new(OPROTO, $2, Z);
        $$->type = $1;
    }
|   '.' '.' '.'          { $$ = new(ODOTDOT, Z, Z); }
|   arglist ',' arglist  { $$ = new(OLIST, $1, $3); }
/*x: parameter declarator rules */
/*
 * parameter declarator
 */
pdecl:
  /* empty */
|   pdecl ctlist pdlist ';'

pdlist:
    xdecor              { dodecl(pdecl, lastclass, lasttype, $1); }
|   pdlist ',' pdlist
/*e: parameter declarator rules */
/*s: abstract declarator rules */
/*
 * abstract declarator
 */
abdecor:
  /* empty */ { $$ = Z; }
|   abdecor1

abdecor1:
    '*' zgnlist
    {
        $$ = new(OIND, (Z), Z);
        $$->garb = simpleg($2);
    }
|   '*' zgnlist abdecor1
    {
        $$ = new(OIND, $3, Z);
        $$->garb = simpleg($2);
    }
|   abdecor2

abdecor2:
    abdecor3
|   abdecor2 '(' zarglist ')'   { $$ = new(OFUNC, $1, $3); }
|   abdecor2 '[' zexpr ']'      { $$ = new(OARRAY, $1, $3); }

abdecor3:
    '(' ')'           { $$ = new(OFUNC, Z, Z); }
|   '[' zexpr ']'     { $$ = new(OARRAY, Z, $2); }
|   '(' abdecor1 ')'  { $$ = $2; }
/*e: abstract declarator rules */
/*s: structure element declarator rules */
/*
 * structure element declarator
 */
edecl:
    tlist
    {
        lasttype = $1;
    }
    zedlist ';'
|   edecl tlist
    {
        lasttype = $2;
    }
    zedlist ';'

zedlist:                    /* extension */
 /* empty */
    {
        lastfield = 0;
        edecl(CXXX, lasttype, S);
    }
|   edlist

edlist:
    edecor            { dodecl(edecl, CXXX, lasttype, $1); }
|   edlist ',' edlist
/*x: structure element declarator rules */
/*s: edecor rule */
edecor:
    xdecor
    {
        lastbit = 0;
        firstbit = 1;
    }
/*x: edecor rule */
|   tag ':' lexpr   { $$ = new(OBIT, $1, $3); }
|   ':' lexpr       { $$ = new(OBIT, Z, $2); }
/*e: edecor rule */
/*e: structure element declarator rules */
/*e: declarator rules */
/*s: statements rules */
block:
 '{' slist '}'
    {
        $$ = invert($2);
        if($$ == Z)
            $$ = new(OLIST, Z, Z);
    }

slist:
  /* empty */      { $$ = Z;    }
|   slist adecl    { $$ = new(OLIST, $1, $2); }
|   slist stmnt    { $$ = new(OLIST, $1, $2); }
/*x: statements rules */
stmnt:
    ulstmnt
|   labels ulstmnt { $$ = new(OLIST, $1, $2); }
|   error ';'      { $$ = Z; }
/*x: statements rules */
/*s: ulstmnt rule */
ulstmnt:
    zcexpr ';'
/*x: ulstmnt rule */
|   {
        markdcl();
    }
    block
    {
        $$ = revertdcl();
        if($$)
            $$ = new(OLIST, $$, $2);
        else
            $$ = $2;
    }
/*x: ulstmnt rule */
|   LIF '(' cexpr ')' stmnt
    {
        $$ = new(OIF, $3, new(OLIST, $5, Z));
        if($5 == Z)
            warn($3, "empty if body");
    }
|   LIF '(' cexpr ')' stmnt LELSE stmnt
    {
        $$ = new(OIF, $3, new(OLIST, $5, $7));
        if($5 == Z)
            warn($3, "empty if body");
        if($7 == Z)
            warn($3, "empty else body");
    }
/*x: ulstmnt rule */
|   { 
        markdcl(); 
    } 
    LFOR '(' forexpr ';' zcexpr ';' zcexpr ')' stmnt
    {
        $$ = revertdcl();
        if($$){
            if($4)
                $4 = new(OLIST, $$, $4);
            else
                $4 = $$;
        }
        $$ = new(OFOR, new(OLIST, $6, new(OLIST, $4, $8)), $10);
    }
|   LWHILE '(' cexpr ')' stmnt          { $$ = new(OWHILE, $3, $5); }
|   LDO stmnt LWHILE '(' cexpr ')' ';'  { $$ = new(ODWHILE, $5, $2); }
/*x: ulstmnt rule */
|   LRETURN zcexpr ';'
    {
        $$ = new(ORETURN, $2, Z);
        $$->type = thisfn->link;
    }
|   LBREAK ';'     { $$ = new(OBREAK, Z, Z); }
|   LCONTINUE ';'  { $$ = new(OCONTINUE, Z, Z); }
/*x: ulstmnt rule */
|   LSWITCH '(' cexpr ')' stmnt
    {
       /*s: ulstmt rule, SWITCH case, adjust cexpr node */
       // generate (0:int - (0:int - x))
       // which will force the usual arithmetic conversions
       // (and will be simplified later by some transformations)

       $$ = new(OCONST, Z, Z);
       $$->vconst = 0;
       $$->type = types[TINT];
       $3 = new(OSUB, $$, $3);

       $$ = new(OCONST, Z, Z);
       $$->vconst = 0;
       $$->type = types[TINT];
       $3 = new(OSUB, $$, $3);
       /*e: ulstmt rule, SWITCH case, adjust cexpr node */
        $$ = new(OSWITCH, $3, $5);
    }
/*x: ulstmnt rule */
|   LGOTO ltag ';' { $$ = new(OGOTO, dcllabel($2, false), Z); }
/*x: ulstmnt rule */
|   LUSED '(' zelist ')' ';' { $$ = new(OUSED, $3, Z); }
|   LSET '(' zelist ')' ';'  { $$ = new(OSET, $3, Z); }
/*e: ulstmnt rule */
/*s: label rule */
label:
    LCASE expr ':'  { $$ = new(OCASE, $2, Z); }
|   LDEFAULT ':'    { $$ = new(OCASE, Z, Z); }
/*x: label rule */
|   LNAME ':'       { $$ = new(OLABEL, dcllabel($1, true), Z); }
/*e: label rule */
/*x: statements rules */
forexpr:
    zcexpr
|   ctlist adlist { $$ = $2; }
/*e: statements rules */
/*s: expressions rules */
/*s: expr rule */
expr:
    xuexpr
/*x: expr rule */
|   expr '+' expr { $$ = new(OADD, $1, $3); }
|   expr '-' expr { $$ = new(OSUB, $1, $3); }
|   expr '*' expr { $$ = new(OMUL, $1, $3); }
|   expr '/' expr { $$ = new(ODIV, $1, $3); }
|   expr '%' expr { $$ = new(OMOD, $1, $3); }
|   expr LRSH expr { $$ = new(OASHR, $1, $3); }
|   expr LLSH expr { $$ = new(OASHL, $1, $3); }
|   expr '&' expr  { $$ = new(OAND, $1, $3); }
|   expr '^' expr  { $$ = new(OXOR, $1, $3); }
|   expr '|' expr  { $$ = new(OOR, $1, $3); }
/*x: expr rule */
|   expr LANDAND expr { $$ = new(OANDAND, $1, $3); }
|   expr LOROR expr   { $$ = new(OOROR, $1, $3); }
/*x: expr rule */
|   expr LEQ expr  { $$ = new(OEQ, $1, $3); }
|   expr LNE expr  { $$ = new(ONE, $1, $3); }
|   expr '<' expr  { $$ = new(OLT, $1, $3); }
|   expr '>' expr  { $$ = new(OGT, $1, $3); }
|   expr LLE expr  { $$ = new(OLE, $1, $3); }
|   expr LGE expr  { $$ = new(OGE, $1, $3); }
/*x: expr rule */
|   expr '=' expr  { $$ = new(OAS, $1, $3); }
|   expr LPE expr  { $$ = new(OASADD, $1, $3); }
|   expr LME expr  { $$ = new(OASSUB, $1, $3); }
|   expr LMLE expr { $$ = new(OASMUL, $1, $3); }
|   expr LDVE expr { $$ = new(OASDIV, $1, $3); }
|   expr LMDE expr { $$ = new(OASMOD, $1, $3); }
|   expr LLSHE expr { $$ = new(OASASHL, $1, $3); }
|   expr LRSHE expr { $$ = new(OASASHR, $1, $3); }
|   expr LANDE expr { $$ = new(OASAND, $1, $3); }
|   expr LXORE expr { $$ = new(OASXOR, $1, $3); }
|   expr LORE expr  { $$ = new(OASOR, $1, $3); }
/*x: expr rule */
|   expr '?' cexpr ':' expr { $$ = new(OCOND, $1, new(OLIST, $3, $5)); }
/*e: expr rule */
/*s: xuexpr rule */
xuexpr:
    uexpr
/*x: xuexpr rule */
|   '(' tlist abdecor ')' xuexpr
    {
        $$ = new(OCAST, $5, Z);
        dodecl(NODECL, CXXX, $2, $3);
        $$->type = lastdcl;
        $$->xcast = true;
    }
/*x: xuexpr rule */
|   '(' tlist abdecor ')' '{' ilist '}' /* extension */
    {
        $$ = new(OSTRUCT, $6, Z);
        dodecl(NODECL, CXXX, $2, $3);
        $$->type = lastdcl;
    }
/*e: xuexpr rule */
/*s: uexpr rule */
uexpr:
    pexpr
/*x: uexpr rule */
|   '+' xuexpr { $$ = new(OPOS, $2, Z); }
|   '-' xuexpr { $$ = new(ONEG, $2, Z); }
/*x: uexpr rule */
|   '!' xuexpr { $$ = new(ONOT, $2, Z); }
|   '~' xuexpr { $$ = new(OCOM, $2, Z); }
/*x: uexpr rule */
|   '*' xuexpr { $$ = new(OIND, $2, Z); }
|   '&' xuexpr { $$ = new(OADDR, $2, Z); }
/*x: uexpr rule */
|   LPP xuexpr { $$ = new(OPREINC, $2, Z); }
|   LMM xuexpr { $$ = new(OPREDEC, $2, Z); }
/*x: uexpr rule */
|   LSIZEOF uexpr { $$ = new(OSIZE, $2, Z); }
/*x: uexpr rule */
|   LSIGNOF uexpr { $$ = new(OSIGN, $2, Z); }
/*e: uexpr rule */
/*s: pexpr rule */
pexpr:
    '(' cexpr ')' { $$ = $2; }
/*x: pexpr rule */
|   pexpr '(' zelist ')'
    {
        $$ = new(OFUNC, $1, Z);
        if($1->op == ONAME)
          if($1->type == T)
            dodecl(xdecl, CXXX, types[TINT], $$);
        $$->right = invert($3);
    }
/*x: pexpr rule */
|   pexpr '[' cexpr ']' { $$ = new(OIND, new(OADD, $1, $3), Z); }
/*x: pexpr rule */
|   pexpr '.' ltag
    {
        $$ = new(ODOT, $1, Z);
        $$->sym = $3;
    }
|   pexpr LMG ltag
    {
        $$ = new(ODOT, new(OIND, $1, Z), Z);
        $$->sym = $3;
    }
/*x: pexpr rule */
|   name
/*x: pexpr rule */
|   LCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TINT];
        $$->vconst = $1;
        $$->cstring = strdup(symb);
    }
|   LLCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TLONG];
        $$->vconst = $1;
        $$->cstring = strdup(symb);
    }
|   LVLCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TVLONG];
        $$->vconst = $1;
        $$->cstring = strdup(symb);
    }
/*x: pexpr rule */
|   LUCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TUINT];
        $$->vconst = $1;
        $$->cstring = strdup(symb);
    }
|   LULCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TULONG];
        $$->vconst = $1;
        $$->cstring = strdup(symb);
    }
|   LUVLCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TUVLONG];
        $$->vconst = $1;
        $$->cstring = strdup(symb);
    }
/*x: pexpr rule */
|   LFCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TFLOAT];
        $$->fconst = $1;
        $$->cstring = strdup(symb);
    }
|   LDCONST
    {
        $$ = new(OCONST, Z, Z);
        $$->type = types[TDOUBLE];
        $$->fconst = $1;
        $$->cstring = strdup(symb);
    }
/*x: pexpr rule */
|   string
|   lstring
/*x: pexpr rule */
|   pexpr LPP { $$ = new(OPOSTINC, $1, Z); }
|   pexpr LMM { $$ = new(OPOSTDEC, $1, Z); }
/*x: pexpr rule */
|   LSIZEOF '(' tlist abdecor ')'
    {
        $$ = new(OSIZE, Z, Z);
        dodecl(NODECL, CXXX, $3, $4);
        $$->type = lastdcl;
    }
/*x: pexpr rule */
|   LSIGNOF '(' tlist abdecor ')'
    {
        $$ = new(OSIGN, Z, Z);
        dodecl(NODECL, CXXX, $3, $4);
        $$->type = lastdcl;
    }
/*e: pexpr rule */
/*x: expressions rules */
cexpr:
    expr
|   cexpr ',' cexpr { $$ = new(OCOMMA, $1, $3); }
/*x: expressions rules */
lexpr:
    expr
    {
        $$ = new(OCAST, $1, Z);
        $$->type = types[TLONG];
    }
/*x: expressions rules */
zexpr:
  /* empty */ { $$ = Z; }
|   lexpr
/*x: expressions rules */
elist:
    expr
|   elist ',' elist { $$ = new(OLIST, $1, $3); }
/*x: expressions rules */
string:
    LSTRING
    {
        $$ = new(OSTRING, Z, Z);
        $$->type = typ(TARRAY, types[TCHAR]);
        $$->etype = TARRAY;
        $$->type->width = $1.l + 1;
        $$->cstring = $1.s;
        $$->sym = symstring;
        $$->class = CSTATIC;
    }
|   string LSTRING
    {
        char *s;
        int n;

        n = $1->type->width - 1;
        s = alloc(n+$2.l+MAXALIGN);

        memcpy(s, $1->cstring, n);
        memcpy(s+n, $2.s, $2.l);
        s[n+$2.l] = '\0';

        $$ = $1;
        $$->type->width += $2.l;
        $$->cstring = s;
    }
/*x: expressions rules */
lstring:
    LLSTRING
    {
        $$ = new(OLSTRING, Z, Z);
        $$->type = typ(TARRAY, types[TRUNE]);
        $$->etype = TARRAY;
        $$->type->width = $1.l + sizeof(TRune);
        $$->rstring = (TRune*)$1.s;
        $$->sym = symstring;
        $$->class = CSTATIC;
    }
|   lstring LLSTRING
    {
        char *s;
        int n;

        n = $1->type->width - sizeof(TRune);
        s = alloc(n+$2.l+MAXALIGN);

        memcpy(s, $1->rstring, n);
        memcpy(s+n, $2.s, $2.l);
        *(TRune*)(s+n+$2.l) = 0;

        $$ = $1;
        $$->type->width += $2.l;
        $$->rstring = (TRune*)s;
    }
/*e: expressions rules */
/*s: initializers rules */
init:
    expr
|   '{' ilist '}' { $$ = new(OINIT, invert($2), Z); }

ilist:
    qlist
|   init
|   qlist init { $$ = new(OLIST, $1, $2); }

qlist:
    init ','
|   qlist init ','  { $$ = new(OLIST, $1, $2); }
|   qual
|   qlist qual      { $$ = new(OLIST, $1, $2); }
/*x: initializers rules */
qual:
    '[' lexpr ']' { $$ = new(OARRAY, $2, Z); }
|   '.' ltag
    {
        $$ = new(OELEM, Z, Z);
        $$->sym = $2;
    }
|   qual '='
/*e: initializers rules */
/*s: types rules */
tname:  /* type words */
    LCHAR     { $$ = BCHAR; }
|   LSHORT    { $$ = BSHORT; }
|   LINT      { $$ = BINT; }
|   LLONG     { $$ = BLONG; }
|   LSIGNED   { $$ = BSIGNED; }
|   LUNSIGNED { $$ = BUNSIGNED; }
|   LFLOAT    { $$ = BFLOAT; }
|   LDOUBLE   { $$ = BDOUBLE; }
|   LVOID     { $$ = BVOID; }
/*x: types rules */
cname:  /* class words */
    LAUTO     { $$ = BAUTO; }
|   LSTATIC   { $$ = BSTATIC; }
|   LEXTERN   { $$ = BEXTERN; }
|   LTYPEDEF  { $$ = BTYPEDEF; }
|   LTYPESTR  { $$ = BTYPESTR; }
|   LREGISTER { $$ = BREGISTER; }
|   LINLINE   { $$ = 0; }
/*x: types rules */
gname:  /* garbage words */
    LCONSTNT  { $$ = BCONSTNT; }
|   LVOLATILE { $$ = BVOLATILE; }
|   LRESTRICT { $$ = 0; }
/*x: types rules */
/*s: types rule */
types:
   tname
    {
        $$.t = simplet($1);
        $$.c = CXXX;
    }
/*x: types rule */
|   gcnlist
    {
        $$.t = simplet($1);
        $$.c = simplec($1);
        $$.t = garbt($$.t, $1);
    }
|   tname gctnlist
    {
        $$.t = simplet(typebitor($1, $2));
        $$.c = simplec($2);
        $$.t = garbt($$.t, $2);
    }
|   gcnlist tname
    {
        $$.t = simplet($2);
        $$.c = simplec($1);
        $$.t = garbt($$.t, $1);
    }
|   gcnlist tname gctnlist
    {
        $$.t = simplet(typebitor($2, $3));
        $$.c = simplec($1|$3);
        $$.t = garbt($$.t, $1|$3);
    }
/*x: types rule */
|  complex
    {
        $$.t = $1;
        $$.c = CXXX;
    }
|  complex gctnlist
    {
        $$.t = $1;
        $$.c = simplec($2);
        $$.t = garbt($$.t, $2);
        if($2 & ~BCLASS & ~BGARB)
            diag(Z, "duplicate types given: %T and %Q", $1, $2);
    }
|   gcnlist complex zgnlist
    {
        $$.t = $2;
        $$.c = simplec($1);
        $$.t = garbt($$.t, $1|$3);
    }
/*x: types rule */
sbody:
    '{'
    {
        $<tyty>$.t1 = strf;
        $<tyty>$.t2 = strl;
        $<tyty>$.t3 = lasttype;
        $<tyty>$.c = lastclass;
        strf = T;
        strl = T;
        lastbit = 0;
        firstbit = 1;
        lastclass = CXXX;
        lasttype = T;
    }
    edecl 
    '}'
    {
        $$ = strf;
        strf = $<tyty>2.t1;
        strl = $<tyty>2.t2;
        lasttype = $<tyty>2.t3;
        lastclass = $<tyty>2.c;
    }

/*e: types rule */
/*x: types rules */
gctnlist:
    gctname
|   gctnlist gctname { $$ = typebitor($1, $2); }

zgnlist:
 /* empty */       { $$ = 0; }
|   zgnlist gname  { $$ = typebitor($1, $2); }

gcnlist:
    gcname
|   gcnlist gcname { $$ = typebitor($1, $2); }
/*x: types rules */
/*s: complex rule */
complex:
    LSTRUCT ltag
    {
        dotag($2, TSTRUCT, 0);
        $$ = $2->suetag;
    }
|   LUNION ltag
    {
        dotag($2, TUNION, 0);
        $$ = $2->suetag;
    }
/*x: complex rule */
|   LSTRUCT ltag
    {
        dotag($2, TSTRUCT, autobn);
    }
    sbody
    {
        $$ = $2->suetag;
        if($$->link != T)
            diag(Z, "redeclare tag: %s", $2->name);
        $$->link = $4;
        sualign($$);
    }
|   LUNION ltag
    {
        dotag($2, TUNION, autobn);
    }
    sbody
    {
        $$ = $2->suetag;
        if($$->link != T)
            diag(Z, "redeclare tag: %s", $2->name);
        $$->link = $4;
        sualign($$);
    }
/*x: complex rule */
|   LSTRUCT sbody
    {
        taggen++;
        sprint(symb, "_%d_", taggen);
        $$ = dotag(lookup(), TSTRUCT, autobn);
        $$->link = $2;
        sualign($$);
    }
|   LUNION sbody
    {
        taggen++;
        sprint(symb, "_%d_", taggen);
        $$ = dotag(lookup(), TUNION, autobn);
        $$->link = $2;
        sualign($$);
    }
/*x: complex rule */
|   LENUM ltag
    {
        dotag($2, TENUM, 0);
        $$ = $2->suetag;
        if($$->link == T)
            $$->link = types[TINT];
        $$ = $$->link;
    }
|   LENUM ltag
    {
        dotag($2, TENUM, autobn);
    }
    '{'
    {
        en.tenum = T;
        en.cenum = T;
    }
    enum 
     '}'
    {
        $$ = $2->suetag;
        if($$->link != T)
            diag(Z, "redeclare tag: %s", $2->name);
        if(en.tenum == T) {
            diag(Z, "enum type ambiguous: %s", $2->name);
            en.tenum = types[TINT];
        }
        $$->link = en.tenum;
        $$ = en.tenum;
    }
|   LENUM '{'
    {
        en.tenum = T;
        en.cenum = T;
    }
    enum '}'
    {
        $$ = en.tenum;
    }
/*x: complex rule */
|   LTYPE { $$ = tcopy($1->type); }
/*e: complex rule */
/*x: types rules */
enum:
    LNAME           { doenum($1, Z); }
|   LNAME '=' expr  { doenum($1, $3); }
|   enum ','
|   enum ',' enum
/*e: types rules */
/*s: names rules */
name:
    LNAME
    {
        $$ = new(ONAME, Z, Z);
        if($1->class == CLOCAL)
            $1 = mkstatic($1);
        $$->sym = $1;
        $$->type = $1->type;

        $$->etype = TVOID;
        if($$->type != T)
            $$->etype = $$->type->etype;

        $$->xoffset = $1->offset;
        $$->class = $1->class;

        $1->aused = true;
    }
/*x: names rules */
gctname:
    tname
|   gname
|   cname

gcname:
    gname
|   cname
/*x: names rules */
tag:
    ltag
    {
        $$ = new(ONAME, Z, Z);
        $$->sym = $1;

        $$->type = $1->type;
        $$->etype = TVOID;
        if($$->type != T)
            $$->etype = $$->type->etype;

        $$->xoffset = $1->offset;
        $$->class = $1->class;
    }
/*x: names rules */
ltag:
    LNAME
|   LTYPE
/*e: names rules */

/*s: extra grammar rules */
tlist:
    types
    {
        $$ = $1.t;
        if($1.c != CXXX)
            diag(Z, "illegal combination of class 4: %s", cnames[$1.c]);
    }
/*x: extra grammar rules */
ctlist:
    types
    {
        lasttype = $1.t;
        lastclass = $1.c;
    }
/*e: extra grammar rules */
/*s: ebnf grammar rules */
labels:
    label
|   labels label  { $$ = new(OLIST, $1, $2); }
/*x: ebnf grammar rules */
zcexpr:
  /* empty */ { $$ = Z; }
|   cexpr
/*x: ebnf grammar rules */
zelist:
  /* empty */ { $$ = Z; }
|   elist
/*x: ebnf grammar rules */
zctlist:
 /* empty */
    {
        lastclass = CXXX;
        lasttype = types[TINT];
    }
|   ctlist
/*e: ebnf grammar rules */
/*e: grammar */
%%
/*e: cc/cc.y */
