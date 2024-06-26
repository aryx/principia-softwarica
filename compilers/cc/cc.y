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
   Node*   node;
   /*x: [[union yacc]] other fields */
   long    lval; //bitset<enum<Bxxx> >
   /*x: [[union yacc]] other fields */
   Type*   type;
   /*x: [[union yacc]] other fields */
   struct
   {
       Type*   t;
       // option<enum<Storage_class> >, None = CXXX
       byte   c;
   } tycl;
   /*x: [[union yacc]] other fields */
   struct
   {
       Type*   t1; // save strf
       Type*   t2; // save strl
       Type*   t3; // save lastype
       uchar   c;  // save lastclass
   } tyty;
   /*e: [[union yacc]] other fields */
}
/*e: union yacc */
/*s: token declarations */
%token  <sym>   LNAME LTYPE
%token  <vval>  LCONST LUCONST  LLCONST  LULCONST   LVLCONST LUVLCONST
%token  <dval>  LFCONST LDCONST
%token  <sval>  LSTRING LLSTRING
/*x: token declarations */
%token  LVOID   LCHAR LSHORT LINT LLONG   LDOUBLE LFLOAT   LSIGNED LUNSIGNED
%token  LSTRUCT LUNION LENUM
%token  LTYPEDEF  
%token  LCONSTNT LVOLATILE  LRESTRICT LINLINE
%token  LAUTO LSTATIC LEXTERN LREGISTER
%token  LIF LELSE  LWHILE LDO LFOR  LBREAK LCONTINUE  LRETURN LGOTO
%token  LSWITCH LCASE LDEFAULT 
%token  LSIZEOF
/*x: token declarations */
%token  LUSED LSET 
/*x: token declarations */
%token  LTYPESTR
/*x: token declarations */
%token  LSIGNOF
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
/*s: type [[declarations]] */
%type   <node>  name tag 
%type   <node>  stmnt   block slist ulstmnt labels label forexpr
%type   <node>  expr    cexpr lexpr  xuexpr uexpr pexpr  elist  string lstring
%type   <node>  adecl   qual init ilist  adlist qlist  arglist
%type   <node>  xdecor xdecor2 edecor  abdecor abdecor1 abdecor2 abdecor3
%type   <node>  zexpr zelist zcexpr zarglist
/*x: type [[declarations]] */
%type   <sym>   ltag
/*x: type [[declarations]] */
%type   <lval>  tname cname gname
/*x: type [[declarations]] */
%type   <lval>   gcname   gctname
/*x: type [[declarations]] */
%type   <type> tlist
/*x: type [[declarations]] */
%type   <tycl>  types
/*x: type [[declarations]] */
%type   <lval>  gctnlist gcnlist zgnlist
/*x: type [[declarations]] */
%type   <type>  complex
/*x: type [[declarations]] */
%type   <type> sbody
/*e: type [[declarations]] */
%%
/*s: grammar */
prog:
  /* empty */
|   prog xdecl

/*s: declarator rules */
/*s: external declarator rules */
/*s: xdecl rule */
/*
 * external declarator
 */
xdecl:
    zctlist ';'          { dodecl(xdecl, lastclass, lasttype, Z); }
/*x: xdecl rule */
|   zctlist xdlist ';'
/*x: xdecl rule */
|   zctlist xdecor
    {
        lastdcltype = T; //dead??
        /*s: xdecl rule, initializations before processing a function */
        firstarg = S; // can mv after call to dodecl? No I think.
        /*e: xdecl rule, initializations before processing a function */
        dodecl(xdecl, lastclass, lasttype, $2);
        /*s: xdecl rule, sanity check lastdcltype is a function type */
        if(lastdcltype == T || lastdcltype->etype != TFUNC) {
            diag($2, "not a function");
            lastdcltype = types[TFUNC];
        }
        /*e: xdecl rule, sanity check lastdcltype is a function type */
        thisfntype = lastdcltype;
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
        /*s: xdecl rule, adjust block body with possible hidden generated nodes */
        if(n)
            $6 = new(OLIST, n, $6);
        /*e: xdecl rule, adjust block body with possible hidden generated nodes */
        /*s: xdecl rule, debug function body */
        if(debug['x']) {
            prtree($2, "func");
            prtree($6, "body");
        }
        /*e: xdecl rule, debug function body */
        if(!debug['a'] && !debug['Z'])
            codgen($6, $2); // !!!!!!!!!!!!!!!!!!!!!
    }
/*e: xdecl rule */
/*s: xdlist rule */
xdlist:
    xdecor  { dodecl(xdecl, lastclass, lasttype, $1); }
|   xdlist ',' xdlist
/*x: xdlist rule */
|   xdecor
    {
        $1 = dodecl(xdecl, lastclass, lasttype, $1);
    }
    '=' init
    {
        doinit($1->sym, $1->type, 0L, $4);
    }
/*e: xdlist rule */
/*s: xdecor rule */
xdecor:
    xdecor2
/*x: xdecor rule */
|   '*' zgnlist xdecor
    {
        $$ = new(OIND, $3, Z);
        $$->nodegarb = simpleg($2);
    }
/*e: xdecor rule */
/*s: xdecor2 rule */
xdecor2:
    tag
|   '(' xdecor ')'             { $$ = $2;   }
/*x: xdecor2 rule */
|   xdecor2 '[' zexpr ']'      { $$ = new(OARRAY, $1, $3); }
/*x: xdecor2 rule */
|   xdecor2 '(' zarglist ')'   { $$ = new(OFUNC, $1, $3); }
/*e: xdecor2 rule */
/*e: external declarator rules */
/*s: automatic declarator rules */
/*
 * automatic declarator
 */
adecl:
    ctlist ';'        { $$ = dodecl(adecl, lastclass, lasttype, Z); }
|   ctlist adlist ';' { $$ = $2; }
/*x: automatic declarator rules */
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
        /*s: adlist rule, after doinit */
        $$ = contig($1->sym, $$, w);
        /*e: adlist rule, after doinit */
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
/*x: parameter declarator rules */
/*s: arglist rule */
arglist:
    name
/*x: arglist rule */
|   tlist xdecor
    {
        $$ = new(OPROTO, $2, Z);
        $$->type = $1;
    }
|   tlist abdecor
    {
        $$ = new(OPROTO, $2, Z);
        $$->type = $1;
    }
/*x: arglist rule */
|   arglist ',' arglist  { $$ = new(OLIST, $1, $3); }
/*x: arglist rule */
|   '.' '.' '.'          { $$ = new(ODOTDOT, Z, Z); }
/*e: arglist rule */
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
/*x: abstract declarator rules */
abdecor1:
    '*' zgnlist
    {
        $$ = new(OIND, (Z), Z);
        $$->nodegarb = simpleg($2);
    }
|   '*' zgnlist abdecor1
    {
        $$ = new(OIND, $3, Z);
        $$->nodegarb = simpleg($2);
    }
|   abdecor2
/*x: abstract declarator rules */
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

/*s: zedlist rule */
zedlist:                    /* extension */
 /* empty */
    {
        lastfield = 0;
        edecl(CXXX, lasttype, S);
    }
|   edlist
/*e: zedlist rule */

edlist:
    edecor            { dodecl(edecl, CXXX, lasttype, $1); }
|   edlist ',' edlist
/*x: structure element declarator rules */
/*s: edecor rule */
edecor:
    xdecor
    {
        /*s: edecor rule, set fields after parsed a field declarator */
        lastbit = 0;
        firstbit = true;
        /*e: edecor rule, set fields after parsed a field declarator */
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
        Node *n;
        n = revertdcl();
        /*s: ulstmt rule, adjust block body with possible hidden generated nodes */
        if(n)
            $$ = new(OLIST, n, $2);
        /*e: ulstmt rule, adjust block body with possible hidden generated nodes */
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
|   LWHILE '(' cexpr ')' stmnt          { $$ = new(OWHILE, $3, $5); }
|   LDO stmnt LWHILE '(' cexpr ')' ';'  { $$ = new(ODWHILE, $5, $2); }
/*x: ulstmnt rule */
|   { 
        markdcl(); 
    } 
    LFOR '(' forexpr ';' zcexpr ';' zcexpr ')' stmnt
    {
        Node *n;
        n = revertdcl();
        /*s: ulstmt rule, adjust forexpr with possible hidden generated nodes */
        if(n){
            if($4)
                $4 = new(OLIST, n, $4);
            else
                $4 = n;
        }
        /*e: ulstmt rule, adjust forexpr with possible hidden generated nodes */
        $$ = new(OFOR, new(OLIST, $6, new(OLIST, $4, $8)), $10);
    }
/*x: ulstmnt rule */
|   LRETURN zcexpr ';'
    {
        $$ = new(ORETURN, $2, Z);
        $$->type = thisfntype->link;
    }
/*x: ulstmnt rule */
|   LBREAK ';'     { $$ = new(OBREAK, Z, Z); }
|   LCONTINUE ';'  { $$ = new(OCONTINUE, Z, Z); }
/*x: ulstmnt rule */
|   LGOTO ltag ';' { $$ = new(OGOTO, dcllabel($2, false), Z); }
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
|   LUSED '(' zelist ')' ';' { $$ = new(OUSED, $3, Z); }
|   LSET '(' zelist ')' ';'  { $$ = new(OSET, $3, Z); }
/*e: ulstmnt rule */
/*s: label rule */
|   LNAME ':'       { $$ = new(OLABEL, dcllabel($1, true), Z); }
/*x: label rule */
label:
    LCASE expr ':'  { $$ = new(OCASE, $2, Z); }
|   LDEFAULT ':'    { $$ = new(OCASE, Z, Z); }
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
/*x: expr rule */
|   expr '&' expr  { $$ = new(OAND, $1, $3); }
|   expr '^' expr  { $$ = new(OXOR, $1, $3); }
|   expr '|' expr  { $$ = new(OOR, $1, $3); }

|   expr LRSH expr { $$ = new(OASHR, $1, $3); }
|   expr LLSH expr { $$ = new(OASHL, $1, $3); }
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
/*x: expr rule */
|   expr LPE expr  { $$ = new(OASADD, $1, $3); }
|   expr LME expr  { $$ = new(OASSUB, $1, $3); }
|   expr LMLE expr { $$ = new(OASMUL, $1, $3); }
|   expr LDVE expr { $$ = new(OASDIV, $1, $3); }
|   expr LMDE expr { $$ = new(OASMOD, $1, $3); }
|   expr LANDE expr { $$ = new(OASAND, $1, $3); }
|   expr LXORE expr { $$ = new(OASXOR, $1, $3); }
|   expr LORE expr  { $$ = new(OASOR, $1, $3); }
|   expr LLSHE expr { $$ = new(OASASHL, $1, $3); }
|   expr LRSHE expr { $$ = new(OASASHR, $1, $3); }
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
        $$->type = lastdcltype;
        $$->xcast = true;
    }
/*x: xuexpr rule */
|   '(' tlist abdecor ')' '{' ilist '}' /* extension */
    {
        $$ = new(OSTRUCT, $6, Z);
        dodecl(NODECL, CXXX, $2, $3);
        $$->type = lastdcltype;
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
|   name
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
|   pexpr '(' zelist ')'
    {
        $$ = new(OFUNC, $1, Z);
        /*s: pexpr rule, implicit declaration of unknown function */
        if(($1->op == ONAME) && ($1->type == T))
            dodecl(xdecl, CXXX, types[TINT], $$);
        /*e: pexpr rule, implicit declaration of unknown function */
        $$->right = invert($3);
    }
/*x: pexpr rule */
|   pexpr LPP { $$ = new(OPOSTINC, $1, Z); }
|   pexpr LMM { $$ = new(OPOSTDEC, $1, Z); }
/*x: pexpr rule */
|   LSIZEOF '(' tlist abdecor ')'
    {
        $$ = new(OSIZE, Z, Z);
        dodecl(NODECL, CXXX, $3, $4);
        $$->type = lastdcltype;
    }
/*x: pexpr rule */
|   LSIGNOF '(' tlist abdecor ')'
    {
        $$ = new(OSIGN, Z, Z);
        dodecl(NODECL, CXXX, $3, $4);
        $$->type = lastdcltype;
    }
/*e: pexpr rule */
/*x: expressions rules */
cexpr:
    expr
|   cexpr ',' cexpr { $$ = new(OCOMMA, $1, $3); }
/*x: expressions rules */
zexpr:
  /* empty */ { $$ = Z; }
|   lexpr
/*x: expressions rules */
lexpr:
    expr
    {
        $$ = new(OCAST, $1, Z);
        $$->type = types[TLONG];
    }
/*x: expressions rules */
string:
    LSTRING
    {
        $$ = new(OSTRING, Z, Z);
        $$->type = typ(TARRAY, types[TCHAR]);
        $$->etype = TARRAY;
        $$->type->width = $1.l + 1;
        $$->cstring = $1.s;
        /*s: string rule, set sym and class for OSTRING */
        $$->sym = symstring;
        $$->class = CSTATIC;
        /*e: string rule, set sym and class for OSTRING */
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
        /*s: string rule, set sym and class for OSTRING */
        $$->sym = symstring;
        $$->class = CSTATIC;
        /*e: string rule, set sym and class for OSTRING */
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
/*x: expressions rules */
elist:
    expr
|   elist ',' elist { $$ = new(OLIST, $1, $3); }
/*e: expressions rules */
/*s: initializers rules */
init:
    expr
|   '{' ilist '}' { $$ = new(OINIT, invert($2), Z); }
/*x: initializers rules */
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
/*s: types, storage classes, qualifiers rules */
/*s: tname rule */
tname:  /* type words */
    LCHAR     { $$ = BCHAR; }
|   LSHORT    { $$ = BSHORT; }
|   LINT      { $$ = BINT; }
|   LLONG     { $$ = BLONG; }

|   LFLOAT    { $$ = BFLOAT; }
|   LDOUBLE   { $$ = BDOUBLE; }

|   LVOID     { $$ = BVOID; }
/*x: tname rule */
|   LSIGNED   { $$ = BSIGNED; }
|   LUNSIGNED { $$ = BUNSIGNED; }
/*e: tname rule */
/*s: cname rule */
cname:  /* class words */
    LAUTO     { $$ = BAUTO; }
|   LSTATIC   { $$ = BSTATIC; }
|   LEXTERN   { $$ = BEXTERN; }
|   LREGISTER { $$ = BREGISTER; }
|   LINLINE   { $$ = 0; }
/*x: cname rule */
|   LTYPEDEF  { $$ = BTYPEDEF; }
/*x: cname rule */
|   LTYPESTR  { $$ = BTYPESTR; }
/*e: cname rule */
/*s: gname rule */
gname:  /* garbage words */
    LCONSTNT  { $$ = BCONSTNT; }
|   LVOLATILE { $$ = BVOLATILE; }
|   LRESTRICT { $$ = 0; }
/*e: gname rule */
/*x: types, storage classes, qualifiers rules */
gctname:
    tname
|   gname
|   cname

gcname:
    gname
|   cname
/*x: types, storage classes, qualifiers rules */
tlist:
    types
    {
        $$ = $1.t;
        if($1.c != CXXX)
            diag(Z, "illegal combination of class 4: %s", cnames[$1.c]);
    }
/*x: types, storage classes, qualifiers rules */
gctnlist:
    gctname
|   gctnlist gctname { $$ = typebitor($1, $2); }
/*x: types, storage classes, qualifiers rules */
gcnlist:
    gcname
|   gcnlist gcname { $$ = typebitor($1, $2); }
/*x: types, storage classes, qualifiers rules */
/*s: types rule */
types:
   tname
    {
        $$.c = CXXX;
        $$.t = simplet($1);
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
        $$.c = CXXX;
        $$.t = $1;
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
/*e: types rule */
/*x: types, storage classes, qualifiers rules */
zgnlist:
 /* empty */       { $$ = 0; }
|   zgnlist gname  { $$ = typebitor($1, $2); }
/*x: types, storage classes, qualifiers rules */
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
        /*s: complex rule, when parse a structure definition, align the struct */
        sualign($$);
        /*e: complex rule, when parse a structure definition, align the struct */
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
        /*s: complex rule, when parse a structure definition, align the struct */
        sualign($$);
        /*e: complex rule, when parse a structure definition, align the struct */
    }
/*x: complex rule */
|   LSTRUCT sbody
    {
        taggen++;
        sprint(symb, "_%d_", taggen);
        $$ = dotag(lookup(), TSTRUCT, autobn);
        $$->link = $2;
        /*s: complex rule, when parse a structure definition, align the struct */
        sualign($$);
        /*e: complex rule, when parse a structure definition, align the struct */
    }
|   LUNION sbody
    {
        taggen++;
        sprint(symb, "_%d_", taggen);
        $$ = dotag(lookup(), TUNION, autobn);
        $$->link = $2;
        /*s: complex rule, when parse a structure definition, align the struct */
        sualign($$);
        /*e: complex rule, when parse a structure definition, align the struct */
    }
/*x: complex rule */
|   LENUM ltag
    {
        dotag($2, TENUM, 0);
        $$ = $2->suetag;
        if($$->link == T) // default type
            $$->link = types[TINT];
        $$ = $$->link;
    }
/*x: complex rule */
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
        /*s: complex rule, after processed an enum, sanity check tenum */
        if(en.tenum == T) {
            diag(Z, "enum type ambiguous: %s", $2->name);
            en.tenum = types[TINT];
        }
        /*e: complex rule, after processed an enum, sanity check tenum */
        $$->link = en.tenum;
        $$ = en.tenum;
    }
|   LENUM 
    '{'
    {
        en.tenum = T;
        en.cenum = T;
    }
    enum 
    '}'
    {
        $$ = en.tenum;
    }
/*x: complex rule */
|   LTYPE { $$ = tcopy($1->type); }
/*e: complex rule */
/*x: types, storage classes, qualifiers rules */
sbody:
    '{'
    {
        // save
        $<tyty>$.t1 = strf;
        $<tyty>$.t2 = strl;
        $<tyty>$.t3 = lasttype;
        $<tyty>$.c = lastclass;

        strf = T;
        strl = T;
        /*s: sbody rule, initializations before parsing the fields */
        lastbit = 0;
        firstbit = true;
        /*e: sbody rule, initializations before parsing the fields */
        lastclass = CXXX;
        lasttype = T;
    }
    edecl 
    '}'
    {
        $$ = strf;

        // restore
        strf = $<tyty>2.t1;
        strl = $<tyty>2.t2;
        lasttype = $<tyty>2.t3;
        lastclass = $<tyty>2.c;
    }

/*x: types, storage classes, qualifiers rules */
enum:
    LNAME           { doenum($1, Z); }
|   LNAME '=' expr  { doenum($1, $3); }

|   enum ',' enum
|   enum ','
/*e: types, storage classes, qualifiers rules */
/*s: names rules */
tag:
    ltag
    {
        $$ = new(ONAME, Z, Z);
        $$->sym = $1;
        /*s: tag rule, set other fields to name node */
        $$->type = $1->type;
        $$->etype = ($1->type != T) ? $1->type->etype : TVOID;
        $$->class = $1->class;

        $$->xoffset = $1->offset;
        /*e: tag rule, set other fields to name node */
    }
/*x: names rules */
ltag:
    LNAME
|   LTYPE
/*x: names rules */
name:
    LNAME
    {
        $$ = new(ONAME, Z, Z);
        /*s: name rule, if local static variable */
        if($1->class == CLOCAL)
            $1 = mkstatic($1);
        /*e: name rule, if local static variable */
        $$->sym = $1;

        // propagate symbol information to node
        $$->type = $1->type;
        $$->etype = ($1->type != T) ? $1->type->etype : TVOID;
        $$->xoffset = $1->offset;
        $$->class = $1->class;
        /*s: name rule, LNAME case, adjust more fields */
        $1->aused = true;
        /*e: name rule, LNAME case, adjust more fields */
    }
/*e: names rules */

/*s: extra grammar rules */
ctlist:
    types
    {
        lastclass = $1.c;
        lasttype = $1.t;
    }
/*e: extra grammar rules */
/*s: ebnf grammar rules */
zctlist:
 /* empty */
    {
        lastclass = CXXX;
        lasttype = types[TINT];
    }
|   ctlist
/*x: ebnf grammar rules */
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
/*e: ebnf grammar rules */
/*e: grammar */
%%
/*e: cc/cc.y */
