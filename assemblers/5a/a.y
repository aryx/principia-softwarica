/*s: 5a/a.y */
%{
#include "a.h"
%}
/*s: union declaration(arm) */
%union {
 // long for LCONST
 // and enum<Opcode>  for LARITH/...
 // and enum<Register> for LREG/...
 // and enum<Cond>    for LCOND
 // and ...
 long   lval;    // for LCONST/LARITH/LREG/LCOND/...
 double dval;    // for LFCONST
 char   sval[NSNAME]; // for LSCONST

 /*s: union declaration other fields(arm) */
 Sym    *sym;    // for LNAME/LLAB/LVAR
 /*x: union declaration other fields(arm) */
 Gen    genval;
 /*e: union declaration other fields(arm) */
}
/*e: union declaration(arm) */
/*s: priority and associativity declarations */
%left   '|'
%left   '^'
%left   '&'
%left   '<' '>'
%left   '+' '-'
%left   '*' '/' '%'
/*e: priority and associativity declarations */
/*s: token declarations(arm) */
/* opcodes */
%token  <lval>  LARITH LCMP LBRANCH LBCOND LMOV LSWAP LRET
%token  <lval>  LSWI LSYSTEM 
%token  <lval>  LARITHFLOAT LCMPFLOAT LSQRTFLOAT  LMULL LMULA  LMOVM LMVN
%token  <lval>  LDEF LDATA LWORD LEND
%token  <lval>  LMISC
/* registers */
%token  <lval>  LPC LSP LFP LSB
%token  <lval>  LR LREG  LPSR
%token  <lval>  LF LFREG  LFCR 
%token  <lval>  LC LCREG
/* constants */
%token  <lval>  LCONST 
%token  <dval>  LFCONST
%token  <sval>  LSCONST
/* names */
%token  <sym>   LNAME LLAB
%token  <sym>   LVAR
/* bits */
%token  <lval>  LCOND
%token  <lval>  LS LAT
/*e: token declarations(arm) */
/*s: type [[declarations]](arm) */
%type   <genval> imsr
%type   <genval> imm shift reg
/*x: type [[declarations]](arm) */
%type   <genval>   gen 
/*x: type [[declarations]](arm) */
%type   <genval>   ximm
/*x: type [[declarations]](arm) */
%type   <lval>  regi
/*x: type [[declarations]](arm) */
%type   <lval>  rcon
/*x: type [[declarations]](arm) */
%type   <genval> ioreg ireg
/*x: type [[declarations]](arm) */
%type   <genval>   name 
%type   <lval>  offset
%type   <lval>  pointer
/*x: type [[declarations]](arm) */
%type   <genval>   nireg 
/*x: type [[declarations]](arm) */
%type   <genval>   rel
/*x: type [[declarations]](arm) */
%type   <lval>  cond
/*x: type [[declarations]](arm) */
%type   <lval>  con expr 
/*x: type [[declarations]](arm) */
%type <genval> freg fcon frcon
/*x: type [[declarations]](arm) */
%type   <genval>   regreg
/*x: type [[declarations]](arm) */
%type   <lval>  reglist
/*x: type [[declarations]](arm) */
%type   <lval>  creg
/*x: type [[declarations]](arm) */
%type   <lval>  oexpr 
/*e: type [[declarations]](arm) */
%%
/*s: grammar(arm) */
/*s: prog rule(arm) */
prog:
  /* empty */
| prog line
/*e: prog rule(arm) */
/*s: line rule(arm) */
line:
  inst ';'
/*x: line rule(arm) */
| ';'
| error ';'
/*x: line rule(arm) */
| LNAME ':'
 {
  $1->type = LLAB;
  $1->value = pc;
 }
  line
/*x: line rule(arm) */
| LLAB ':'
 {
  if($1->value != pc) {
   yyerror("redeclaration of %s", $1->name);
   $1->value = pc;
  }
 }
  line
/*x: line rule(arm) */
| LNAME '=' expr ';'
 {
  $1->type = LVAR;
  $1->value = $3;
 }
| LVAR '=' expr ';'
 {
  if($1->value != $3)
   yyerror("redeclaration of %s", $1->name);
  $1->value = $3;
 }
/*e: line rule(arm) */
/*s: inst rule(arm) */
inst:
/*
 * AND/ORR/ADD/SUB/...
 */
  LARITH cond imsr ',' regi ',' reg { outcode($1, $2, &$3, $5, &$7); }
| LARITH cond imsr ',' reg          { outcode($1, $2, &$3, R_NONE, &$5); }
/*x: inst rule(arm) */
/*
 * MVN
 */
| LMVN cond imsr ',' reg { outcode($1, $2, &$3, R_NONE, &$5); }
/*x: inst rule(arm) */
/*
 * MOVW
 */
| LMOV cond gen ',' gen { outcode($1, $2, &$3, R_NONE, &$5); }
/*x: inst rule(arm) */
/*
 * SWAP
 */
| LSWAP cond reg ',' ireg         { outcode($1, $2, &$5, $3.reg, &$3); }
| LSWAP cond ireg ',' reg         { outcode($1, $2, &$3, $5.reg, &$5); }
| LSWAP cond reg ',' ireg ',' reg { outcode($1, $2, &$5, $3.reg, &$7); }
/*x: inst rule(arm) */
/*
 * B/BL
 */
| LBRANCH cond rel   { outcode($1, $2, &nullgen, R_NONE, &$3); }
| LBRANCH cond nireg { outcode($1, $2, &nullgen, R_NONE, &$3); }
/*x: inst rule(arm) */
/*
 * CMP
 */
| LCMP cond imsr ',' regi { outcode($1, $2, &$3, $5, &nullgen); }
/*x: inst rule(arm) */
/*
 * BEQ/...
 */
| LBCOND rel { outcode($1, Always, &nullgen, R_NONE, &$2); }
/*x: inst rule(arm) */
/*
 * RET
 */
| LRET cond { outcode($1, $2, &nullgen, R_NONE, &nullgen); }
/*x: inst rule(arm) */
/*
 * SWI
 */
| LSWI cond gen { outcode($1, $2, &nullgen, R_NONE, &$3); }
/*x: inst rule(arm) */
/*
 * TEXT/GLOBL
 */
| LDEF name ',' imm         { outcode($1, Always, &$2, R_NONE, &$4); }
| LDEF name ',' con ',' imm { outcode($1, Always, &$2, $4, &$6); }
/*x: inst rule(arm) */
/*
 * DATA
 */
| LDATA name '/' con ',' ximm { outcode($1, Always, &$2, $4, &$6); }
/*x: inst rule(arm) */
/*
 * WORD
 */
| LWORD ximm { outcode($1, Always, &nullgen, R_NONE, &$2); }
/*x: inst rule(arm) */
/*
 * END
 */
| LEND { outcode($1, Always, &nullgen, R_NONE, &nullgen); }
/*x: inst rule(arm) */
/*
 * floating-point coprocessor
 */
| LARITHFLOAT cond frcon ',' freg { outcode($1, $2, &$3, R_NONE, &$5); }
| LARITHFLOAT cond frcon ',' LFREG ',' freg { outcode($1, $2, &$3, $5, &$7); }
| LCMPFLOAT  cond freg ',' freg { outcode($1, $2, &$3, $5.reg, &nullgen); }
| LSQRTFLOAT cond freg ',' freg { outcode($1, $2, &$3, R_NONE, &$5); }
/*x: inst rule(arm) */
/*
 * MULA hi,lo,r1,r2
 */
| LMULA cond reg ',' reg ',' reg ',' regi 
 {
  $7.type = D_REGREG;
  $7.offset = $9;
  outcode($1, $2, &$3, $5.reg, &$7);
 }
/*x: inst rule(arm) */
/*
 * MULL hi,lo,r1,r2
 */
| LMULL cond reg ',' reg ',' regreg { outcode($1, $2, &$3, $5.reg, &$7); }
/*x: inst rule(arm) */
/*
 * MOVM
 */
| LMOVM cond ioreg ',' '[' reglist ']'
 {
  Gen g;

  g = nullgen;
  g.type = D_CONST;
  g.offset = $6;
  outcode($1, $2, &$3, R_NONE, &g);
 }
| LMOVM cond '[' reglist ']' ',' ioreg
 {
  Gen g;

  g = nullgen;
  g.type = D_CONST;
  g.offset = $4;
  outcode($1, $2, &g, R_NONE, &$7);
 }
/*x: inst rule(arm) */
/*
 * MCR MRC
 */
| LSYSTEM cond con ',' expr ',' regi ',' creg ',' creg oexpr
 {
  Gen g;

  g = nullgen;
  g.type = D_CONST;
  g.offset =
   (0xe << 24) |    /* opcode */
   ($1 << 20) |     /* MCR/MRC */
   ($2 << 28) |     /* scond */
   (($3 & 15) << 8) |   /* coprocessor number */
   (($5 & 7) << 21) |   /* coprocessor operation */
   (($7 & 15) << 12) |  /* arm register */
   (($9 & 15) << 16) |  /* Crn */
   (($11 & 15) << 0) |  /* Crm */
   (($12 & 7) << 5) |   /* coprocessor information */
   (1<<4);          /* must be set */ // opcode component
  outcode(AWORD, Always, &nullgen, R_NONE, &g);
 }
/*e: inst rule(arm) */
/*s: operand rules(arm) */
imsr:
  imm
| shift
| reg
/*x: operand rules(arm) */
/*s: gen rule */
gen:
  ximm
| shift
| reg
/*s: more gen rule */
| ioreg
/*x: more gen rule */
| name
/*x: more gen rule */
| con '(' pointer ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.sym = S;
  $$.symkind = $3;
  $$.offset = $1;
 }
/*x: more gen rule */
| freg
/*x: more gen rule */
| LFCR
 {
  $$ = nullgen;
  $$.type = D_FPCR;
  $$.reg = $1;
 }
/*x: more gen rule */
| LPSR
 {
  $$ = nullgen;
  $$.type = D_PSR;
  $$.reg = $1;
 }
/*e: more gen rule */
/*e: gen rule */
/*s: regi rule(arm) */
regi:
  LREG
/*x: regi rule(arm) */
| LR '(' expr ')'
 {
  if($3 < 0 || $3 >= NREG)
      print("register value out of range\n");
  $$ = $3;
 }
/*e: regi rule(arm) */
/*s: ximm rule */
ximm:
  '$' con
 {
  $$ = nullgen;
  $$.type = D_CONST;
  $$.offset = $2;
 }
/*x: ximm rule */
| '$' LSCONST
 {
  $$ = nullgen;
  $$.type = D_SCONST;
  memcpy($$.sval, $2, sizeof($$.sval));
 }
/*x: ximm rule */
| '$' name
 {
  $$ = $2;
  $$.type = D_ADDR;
 }
/*x: ximm rule */
| fcon
/*e: ximm rule */
/*x: operand rules(arm) */
reg:
 regi
 {
  $$ = nullgen;
  $$.type = D_REG;
  $$.reg = $1;
 }
/*x: operand rules(arm) */
imm: '$' con
 {
  $$ = nullgen;
  $$.type = D_CONST;
  $$.offset = $2;
 }
/*x: operand rules(arm) */
shift:
 regi '<' '<' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (0 << 5);
 }
| regi '>' '>' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (1 << 5);
 }
| regi '-' '>' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (2 << 5);
 }
| regi LAT '>' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (3 << 5);
 }
/*x: operand rules(arm) */
ioreg:
  ireg
| con '(' regi ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.reg = $3;
  $$.offset = $1;
 }
/*x: operand rules(arm) */
ireg:
 '(' regi ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.reg = $2;
  $$.offset = 0;
 }
/*x: operand rules(arm) */
nireg:
  name
| ireg
/*x: operand rules(arm) */
/*s: name rule */
name:
  LNAME offset '(' pointer ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.sym = $1;
  $$.symkind = $4;
  $$.offset = $2;
 }
/*x: name rule */
| LNAME '<' '>' offset '(' LSB ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.sym = $1;
  $$.symkind = N_INTERN;
  $$.offset = $4;
 }
/*e: name rule */
/*x: operand rules(arm) */
/*s: rel rule */
rel:
  LLAB offset
 {
  $$ = nullgen;
  $$.type = D_BRANCH;
  $$.sym = $1;
  $$.offset = $1->value + $2;
 }
/*x: rel rule */
| LNAME offset
 {
  $$ = nullgen;
  if(pass == 2)
      yyerror("undefined label: %s", $1->name);
 }
/*x: rel rule */
| con '(' LPC ')'
 {
  $$ = nullgen;
  $$.type = D_BRANCH;
  $$.offset = $1 + pc;
 }
/*e: rel rule */
/*x: operand rules(arm) */
/* for MULL */
regreg:
 '(' regi ',' regi ')'
 {
  $$ = nullgen;
  $$.type = D_REGREG;
  $$.reg = $2;
  $$.offset = $4;
 }
/*e: operand rules(arm) */
/*s: cond rule(arm) */
cond:
  /* empty */ { $$ = Always; }
| cond LCOND  { $$ = ($1 & ~C_SCOND) | $2; }
/*x: cond rule(arm) */
| cond LS    { $$ = $1 | $2; }
/*e: cond rule(arm) */

/*s: advanced topics rules */
/*s: constant expression rules */
/*s: con rule */
con:
  LCONST
| '-' con      { $$ = -$2; }
| '+' con      { $$ = $2; }
| '~' con      { $$ = ~$2; }
/*x: con rule */
| '(' expr ')' { $$ = $2; }
/*x: con rule */
| LVAR         { $$ = $1->value; }
/*e: con rule */
/*s: expr rule */
expr:
  con
| expr '+' expr     { $$ = $1 + $3; }
| expr '-' expr     { $$ = $1 - $3; }
| expr '*' expr     { $$ = $1 * $3; }
| expr '/' expr     { $$ = $1 / $3; }
| expr '%' expr     { $$ = $1 % $3; }

| expr '<' '<' expr { $$ = $1 << $4; }
| expr '>' '>' expr { $$ = $1 >> $4; }

| expr '&' expr     { $$ = $1 & $3; }
| expr '^' expr     { $$ = $1 ^ $3; }
| expr '|' expr     { $$ = $1 | $3; }

/*e: expr rule */
/*e: constant expression rules */
/*s: float rules */
freg:
  LFREG
 {
  $$ = nullgen;
  $$.type = D_FREG;
  $$.reg = $1;
 }
| LF '(' con ')'
 {
  $$ = nullgen;
  $$.type = D_FREG;
  $$.reg = $3;
 }

fcon:
 '$' LFCONST
 {
  $$ = nullgen;
  $$.type = D_FCONST;
  $$.dval = $2;
 }
| '$' '-' LFCONST
 {
  $$ = nullgen;
  $$.type = D_FCONST;
  $$.dval = -$3;
 }

frcon:
  freg
| fcon
/*e: float rules */
/*s: reglist rule */
reglist:
  regi           { $$ = 1 << $1; }
| regi '-' regi
 {
  int i;
  $$=0;
  for(i=$1; i<=$3; i++)
      $$ |= 1<<i;
  for(i=$3; i<=$1; i++)
      $$ |= 1<<i;
 }
| regi ',' reglist { $$ = (1<<$1) | $3; }
/*e: reglist rule */
/*s: creg rule */
creg:
  LCREG
| LC '(' expr ')'
 {
  if($3 < 0 || $3 >= NREG)
      print("register value out of range\n");
  $$ = $3;
 }
/*e: creg rule */
/*s: oexpr rule */
/* for MCR */ 
oexpr:
  /* empty */ { $$ = 0; }
| ',' expr    { $$ = $2; }
/*e: oexpr rule */
/*e: advanced topics rules */

/*s: helper rules(arm) */
rcon:
  regi
 {
  if($1 < 0 || $1 >= NREG)
      print("register value out of range\n");
  $$ = (($1&15) << 8) | (1 << 4);
 }
| con
 {
  if($1 < 0 || $1 >= 32)
      print("shift value out of range\n");
  $$ = ($1&31) << 7;
 }
/*x: helper rules(arm) */
pointer:
  LSB
| LSP
| LFP
/*x: helper rules(arm) */
offset:
 /* empty */ { $$ = 0; }
| '+' con    { $$ = $2; }
| '-' con    { $$ = -$2; }
/*e: helper rules(arm) */
/*e: grammar(arm) */
/*e: 5a/a.y */
