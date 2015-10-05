/*s: 5a/a.y */
%{
#include "a.h"
%}
/*s: union token(arm) */
%union {
 // long for LCONST
 // and enum<opcode>  for LARITH/...
 // and enum<registr> for LREG/...
 // and enum<cond>    for LCOND
 // and ...
 long   lval;

 double dval;    // for LFCONST
 char   sval[8]; // for LSCONST
 Sym    *sym;    // for LNAME/LLAB/LVAR

 /*s: [[Token]] other fields(arm) */
 Gen    gen;
 /*e: [[Token]] other fields(arm) */
}
/*e: union token(arm) */
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
%token  <lval>  LDEF LDATA LWORD LEND
%token  <lval>  LARITH LCMP LBRANCH LBCOND LMOV LSWAP LSWI LRET
%token  <lval>  LSYSTEM LMISC
%token  <lval>  LARITHFLOAT LCMPFLOAT  LMULL LMULA  LMOVM LMVN
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
/*s: type declarations(arm) */
%type   <gen> imsr
/*x: type declarations(arm) */
%type   <gen> imm shift reg
/*x: type declarations(arm) */
%type   <gen>   gen 
/*x: type declarations(arm) */
%type   <gen>   ximm
/*x: type declarations(arm) */
%type   <lval>  sreg spreg 
/*x: type declarations(arm) */
%type   <lval>  rcon
/*x: type declarations(arm) */
%type   <gen> oreg ioreg ireg nireg 
/*x: type declarations(arm) */
%type   <gen>   name 
%type   <lval>  pointer
/*x: type declarations(arm) */
%type   <gen>   rel
%type   <lval>  offset
/*x: type declarations(arm) */
%type   <lval>  con expr 
/*x: type declarations(arm) */
%type   <lval>  cond
/*x: type declarations(arm) */
%type <gen> freg fcon frcon
/*x: type declarations(arm) */
%type   <gen>   regreg
/*x: type declarations(arm) */
%type   <lval>  reglist
/*x: type declarations(arm) */
%type   <lval>  creg
/*x: type declarations(arm) */
%type   <lval>  oexpr 
/*e: type declarations(arm) */
%%
/*s: grammar(arm) */
prog:
  /* empty */
| prog line

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
  if($1->value != pc)
   yyerror("redeclaration of %s", $1->name);
  $1->value = pc;
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
 * AND/OR/ADD/SUB/...
 */
  LARITH cond imsr ',' spreg ',' reg { outcode($1, $2, &$3, $5, &$7); }
| LARITH cond imsr ',' spreg ','     { outcode($1, $2, &$3, $5, &nullgen); }
| LARITH cond imsr ',' reg           { outcode($1, $2, &$3, R_NONE, &$5); }
/*x: inst rule(arm) */
/*
 * CMP
 */
| LCMP cond imsr ',' spreg { outcode($1, $2, &$3, $5, &nullgen); }
/*x: inst rule(arm) */
/*
 * MOVW
 */
| LMOV cond gen ',' gen { outcode($1, $2, &$3, R_NONE, &$5); }
/*x: inst rule(arm) */
/*
 * MVN
 */
| LMVN cond imsr ',' reg { outcode($1, $2, &$3, R_NONE, &$5); }
/*x: inst rule(arm) */
/*
 * SWAP
 */
| LSWAP cond reg ',' ireg ',' reg { outcode($1, $2, &$5, $3.reg, &$7); }
| LSWAP cond reg ',' ireg         { outcode($1, $2, &$5, $3.reg, &$3); }
| LSWAP cond ireg ',' reg         { outcode($1, $2, &$3, $5.reg, &$5); }
/*x: inst rule(arm) */
/*
 * B/BL
 */
| LBRANCH cond rel   { outcode($1, $2, &nullgen, R_NONE, &$3); }
| LBRANCH cond nireg { outcode($1, $2, &nullgen, R_NONE, &$3); }
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
| LSWI cond { outcode($1, $2, &nullgen, R_NONE, &nullgen); }
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
| LMISC cond freg ',' freg { outcode($1, $2, &$3, R_NONE, &$5); }
| LARITHFLOAT cond frcon ',' freg { outcode($1, $2, &$3, R_NONE, &$5); }
| LARITHFLOAT cond frcon ',' LFREG ',' freg { outcode($1, $2, &$3, $5, &$7); }
| LCMPFLOAT cond freg ',' freg { outcode($1, $2, &$3, $5.reg, &nullgen); }
/*x: inst rule(arm) */
/*
 * MULL hi,lo,r1,r2
 */
| LMULL cond reg ',' reg ',' regreg { outcode($1, $2, &$3, $5.reg, &$7); }
/*x: inst rule(arm) */
/*
 * MULA hi,lo,r1,r2
 */
| LMULA cond reg ',' reg ',' reg ',' spreg 
 {
  $7.type = D_REGREG;
  $7.offset = $9;
  outcode($1, $2, &$3, $5.reg, &$7);
 }
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
| LSYSTEM cond con ',' expr ',' spreg ',' creg ',' creg oexpr
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
/*s: cond rule(arm) */
cond:
  /* empty */ { $$ = Always; }
| cond LCOND  { $$ = ($1 & ~C_SCOND) | $2; }
/*x: cond rule(arm) */
| cond LS    { $$ = $1 | $2; }
/*e: cond rule(arm) */
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
/*x: gen rule */
| oreg
/*x: gen rule */
| freg
/*x: gen rule */
| LFCR
 {
  $$ = nullgen;
  $$.type = D_FPCR;
  $$.reg = $1;
 }
/*x: gen rule */
| LPSR
 {
  $$ = nullgen;
  $$.type = D_PSR;
  $$.reg = $1;
 }
/*e: gen rule */
/*s: spreg rule(arm) */
spreg:
  sreg
/*x: spreg rule(arm) */
| LSP { $$ = REGSP; }
/*e: spreg rule(arm) */
/*s: sreg rule(arm) */
sreg:
  LREG
| LR '(' expr ')'
 {
  if($3 < 0 || $3 >= NREG)
      print("register value out of range\n");
  $$ = $3;
 }
/*x: sreg rule(arm) */
| LPC { $$ = REGPC; }
/*e: sreg rule(arm) */
/*x: operand rules(arm) */
/*s: ximm rule */
ximm:
  '$' con
 {
  $$ = nullgen;
  $$.type = D_CONST;
  $$.offset = $2;
 }
/*x: ximm rule */
| '$' name
 {
  $$ = $2;
  $$.type = D_CONST;
 }
/*x: ximm rule */
| '$' LSCONST
 {
  $$ = nullgen;
  $$.type = D_SCONST;
  memcpy($$.sval, $2, sizeof($$.sval));
 }
/*x: ximm rule */
| fcon
/*e: ximm rule */
/*x: operand rules(arm) */
reg:
 spreg
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
 spreg '<' '<' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (0 << 5);
 }
| spreg '>' '>' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (1 << 5);
 }
| spreg '-' '>' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (2 << 5);
 }
| spreg LAT '>' rcon
 {
  $$ = nullgen;
  $$.type = D_SHIFT;
  $$.offset = $1 | $4 | (3 << 5);
 }
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
  $$.symkind = D_STATIC;
  $$.offset = $4;
 }
/*x: name rule */
| con '(' pointer ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.sym = S;
  $$.symkind = $3;
  $$.offset = $1;
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
  $$.type = D_BRANCH;
  $$.sym = $1;
  $$.offset = $2;
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
 '(' spreg ',' spreg ')'
 {
  $$ = nullgen;
  $$.type = D_REGREG;
  $$.reg = $2;
  $$.offset = $4;
 }
/*e: operand rules(arm) */
/*s: helper rules(arm) */
rcon:
  spreg
 {
  if($$ < 0 || $$ >= NREG)
      print("register value out of range\n");
  $$ = (($1&15) << 8) | (1 << 4);
 }
| con
 {
  if($$ < 0 || $$ >= 32)
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

/*s: constant expression rules */
/*s: con rule */
con:
  LCONST
/*x: con rule */
| '-' con      { $$ = -$2; }
| '+' con      { $$ = $2; }
| '~' con      { $$ = ~$2; }
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
/*s: opt rules */
comma:
  /* empty */
| ',' comma
/*x: opt rules */
/* for MCR */ 
oexpr:
  /* empty */ { $$ = 0; }
| ',' expr    { $$ = $2; }
/*e: opt rules */
/*s: misc rules */
/*s: oreg rule */
oreg:
  ioreg
/*x: oreg rule */
| name
/*e: oreg rule */
/*x: misc rules */
ioreg:
  ireg
| con '(' sreg ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.reg = $3;
  $$.offset = $1;
 }
/*x: misc rules */
ireg:
 '(' spreg ')'
 {
  $$ = nullgen;
  $$.type = D_OREG;
  $$.reg = $2;
  $$.offset = 0;
 }
/*x: misc rules */
nireg:
  name
| ireg
/*x: misc rules */
reglist:
  spreg           { $$ = 1 << $1; }
| spreg '-' spreg
 {
  int i;
  $$=0;
  for(i=$1; i<=$3; i++)
      $$ |= 1<<i;
  for(i=$3; i<=$1; i++)
      $$ |= 1<<i;
 }
| spreg comma reglist { $$ = (1<<$1) | $3; }
/*x: misc rules */
creg:
  LCREG
| LC '(' expr ')'
 {
  if($3 < 0 || $3 >= NREG)
      print("register value out of range\n");
  $$ = $3;
 }
/*e: misc rules */
/*e: grammar(arm) */
/*e: 5a/a.y */
