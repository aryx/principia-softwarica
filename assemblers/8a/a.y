/*s: 8a/a.y */
%{
#include "a.h"
%}
/*s: union token(x86) */
%union {
 //   enum<opcode_kind> (for LTYPE/...) 
 // | enum<operand_kind> (for LBREG/...) 
 // | long (for LCONST)
 long   lval;

 double dval;
 char   sval[8];
 Sym    *sym;

 /*s: [[Token]] other fields(x86) */
  Gen   gen;
 /*x: [[Token]] other fields(x86) */
  Gen2  gen2;
 /*e: [[Token]] other fields(x86) */
}
/*e: union token(x86) */
/*s: priority and associativity declarations */
%left   '|'
%left   '^'
%left   '&'
%left   '<' '>'
%left   '+' '-'
%left   '*' '/' '%'
/*e: priority and associativity declarations */
/*s: token declarations(x86) */
%token  <lval>  LTYPE0 LTYPE1 LTYPE2 LTYPE3 LTYPE4
%token  <lval>  LTYPEC LTYPED LTYPEN LTYPER LTYPET LTYPES LTYPEM LTYPEI LTYPEG
%token  <lval>  LFP LPC LSB LSP
%token  <lval>  LBREG LLREG LSREG LFREG

%token  <lval>  LCONST 
%token  <dval>  LFCONST
%token  <sval>  LSCONST
%token  <sym>   LNAME LLAB LVAR
/*e: token declarations(x86) */
/*s: type [[declarations]](x86) */
%type   <gen>   reg imm mem   omem nmem nam
/*x: type [[declarations]](x86) */
%type   <lval>  pointer
/*x: type [[declarations]](x86) */
%type   <gen>  rel
/*x: type [[declarations]](x86) */
%type   <lval>  offset
/*x: type [[declarations]](x86) */
%type   <gen>  rem rim rom 
/*x: type [[declarations]](x86) */
%type   <gen2>  spec1 spec2 spec3 spec4 spec5 spec6 spec7 spec8
/*x: type [[declarations]](x86) */
%type   <lval>  con expr
/*x: type [[declarations]](x86) */
%type   <gen2>  nonnon nonrel nonrem rimnon rimrem remrim
/*e: type [[declarations]](x86) */
%%
/*s: grammar(x86) */
prog:
  /* empty */
| prog line

/*s: line rule(x86) */
line:
  inst ';'
/*x: line rule(x86) */
| ';'
| error ';'
/*x: line rule(x86) */
| LNAME ':'
 {
  $1->type = LLAB;
  $1->value = pc;
 }
 line
| LLAB ':'
 {
  if($1->value != pc)
   yyerror("redeclaration of %s", $1->name);
  $1->value = pc;
 }
 line
/*e: line rule(x86) */
/*s: inst rule(x86) */
inst:
  LTYPE0 nonnon   { outcode($1, &$2); }
| LTYPE1 nonrem   { outcode($1, &$2); }
| LTYPE2 rimnon   { outcode($1, &$2); }
| LTYPE3 rimrem   { outcode($1, &$2); }
| LTYPE4 remrim   { outcode($1, &$2); }
| LTYPER nonrel   { outcode($1, &$2); }
/*x: inst rule(x86) */
| LTYPED spec1    { outcode($1, &$2); }
| LTYPET spec2    { outcode($1, &$2); }
| LTYPEC spec3    { outcode($1, &$2); }
| LTYPEN spec4    { outcode($1, &$2); }
| LTYPES spec5    { outcode($1, &$2); }
| LTYPEM spec6    { outcode($1, &$2); }
| LTYPEI spec7    { outcode($1, &$2); }
| LTYPEG spec8    { outcode($1, &$2); }
/*x: inst rule(x86) */
| LNAME '=' expr
 {
  $1->type = LVAR;
  $1->value = $3;
 }
| LVAR '=' expr
 {
  if($1->value != $3)
   yyerror("redeclaration of %s", $1->name);
  $1->value = $3;
 }
/*e: inst rule(x86) */
/*s: special opcode operands rules(x86) */
spec2:  /* TEXT */
 mem ',' imm
 {
  $$.from = $1;
  $$.to = $3;
 }
|   mem ',' con ',' imm
 {
  $$.from = $1;
  $$.from.scale = $3;
  $$.to = $5;
 }
/*x: special opcode operands rules(x86) */
spec1:  /* DATA */
 nam '/' con ',' imm
 {
  $$.from = $1;
  $$.from.scale = $3;
  $$.to = $5;
 }
/*x: special opcode operands rules(x86) */
spec8:  /* GLOBL */
 mem ',' imm
 {
  $$.from = $1;
  $$.to = $3;
 }
| mem ',' con ',' imm
 {
  $$.from = $1;
  $$.from.scale = $3;
  $$.to = $5;
 }
/*x: special opcode operands rules(x86) */
spec3:  /* JMP/CALL */
 rom
 {
  $$.from = nullgen;
  $$.to = $1;
 }
| ',' rom
 {
  $$.from = nullgen;
  $$.to = $2;
 }
/*x: special opcode operands rules(x86) */
spec4:  /* NOP */
  nonnon
| nonrem
/*x: special opcode operands rules(x86) */
spec5:  /* SHL/SHR */
 rim ',' rem
 {
  $$.from = $1;
  $$.to = $3;
 }
| rim ',' rem ':' LLREG
 {
  $$.from = $1;
  $$.to = $3;
  if($$.from.index != D_NONE)
   yyerror("dp shift with lhs index");
  $$.from.index = $5;
 }
/*x: special opcode operands rules(x86) */
spec6:  /* MOVW/MOVL */
 rim ',' rem
 {
  $$.from = $1;
  $$.to = $3;
 }
|   rim ',' rem ':' LSREG
 {
  $$.from = $1;
  $$.to = $3;
  if($$.to.index != D_NONE)
   yyerror("dp move with lhs index");
  $$.to.index = $5;
 }
/*x: special opcode operands rules(x86) */
spec7: /* IMUL */
 rim
 {
  $$.from = $1;
  $$.to = nullgen;
 }
| rim ','
 {
  $$.from = $1;
  $$.to = nullgen;
 }
| rim ',' rem
 {
  $$.from = $1;
  $$.to = $3;
 }
/*e: special opcode operands rules(x86) */
/*s: operands rules(x86) */
nonnon:
 /* empty */ { $$.from = nullgen; $$.to = nullgen; }
| ','        { $$.from = nullgen; $$.to = nullgen; }

rimrem:
 rim ',' rem { $$.from = $1; $$.to = $3; }

remrim:
 rem ',' rim { $$.from = $1; $$.to = $3; }

rimnon:
  rim       { $$.from = $1; $$.to = nullgen; }
| rim ','   { $$.from = $1; $$.to = nullgen; }

nonrem:
 rem        { $$.from = nullgen; $$.to = $1; }
| ',' rem   { $$.from = nullgen; $$.to = $2; }

nonrel:
 rel        { $$.from = nullgen; $$.to = $1; }
| ',' rel   { $$.from = nullgen; $$.to = $2; }
/*e: operands rules(x86) */
/*s: operand rules(x86) */
reg:
  LBREG { $$ = nullgen; $$.type = $1; }
| LFREG { $$ = nullgen; $$.type = $1; }
| LLREG { $$ = nullgen; $$.type = $1; }
| LSREG { $$ = nullgen; $$.type = $1; }
| LSP   { $$ = nullgen; $$.type = D_SP; }
/*x: operand rules(x86) */
imm:
 '$' con
 {
  $$ = nullgen;
  $$.type = D_CONST;
  $$.offset = $2;
 }
| '$' nam
 {
  $$ = $2;
  $$.index = $2.type;
  $$.type = D_ADDR;
  /*
  if($2.type == D_AUTO || $2.type == D_PARAM)
   yyerror("constant cannot be automatic: %s",
    $2.sym->name);
   */
 }
| '$' LSCONST
 {
  $$ = nullgen;
  $$.type = D_SCONST;
  memcpy($$.sval, $2, sizeof($$.sval));
 }
| '$' LFCONST
 {
  $$ = nullgen;
  $$.type = D_FCONST;
  $$.dval = $2;
 }
| '$' '(' LFCONST ')'
 {
  $$ = nullgen;
  $$.type = D_FCONST;
  $$.dval = $3;
 }
| '$' '-' LFCONST
 {
  $$ = nullgen;
  $$.type = D_FCONST;
  $$.dval = -$3;
 }
/*x: operand rules(x86) */
mem:
  omem
| nmem
/*x: operand rules(x86) */
omem:
 con
 {
  $$ = nullgen;
  $$.type = D_INDIR+D_NONE;
  $$.offset = $1;
 }
| con '(' LLREG ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+$3;
  $$.offset = $1;
 }
| con '(' LSP ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+D_SP;
  $$.offset = $1;
 }
| con '(' LLREG '*' con ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+D_NONE;
  $$.offset = $1;
  $$.index = $3;
  $$.scale = $5;
  checkscale($$.scale);
 }
| con '(' LLREG ')' '(' LLREG '*' con ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+$3;
  $$.offset = $1;
  $$.index = $6;
  $$.scale = $8;
  checkscale($$.scale);
 }
|   '(' LLREG ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+$2;
 }
|   '(' LSP ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+D_SP;
 }
|   con '(' LSREG ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+$3;
  $$.offset = $1;
 }
|   '(' LLREG '*' con ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+D_NONE;
  $$.index = $2;
  $$.scale = $4;
  checkscale($$.scale);
 }
|   '(' LLREG ')' '(' LLREG '*' con ')'
 {
  $$ = nullgen;
  $$.type = D_INDIR+$2;
  $$.index = $5;
  $$.scale = $7;
  checkscale($$.scale);
 }
/*x: operand rules(x86) */
nmem:
  nam
| nam '(' LLREG '*' con ')'
 {
  $$ = $1;
  $$.index = $3;
  $$.scale = $5;
  checkscale($$.scale);
 }
/*x: operand rules(x86) */
nam:
  LNAME offset '(' pointer ')'
 {
  $$ = nullgen;
  $$.type = $4;
  $$.sym = $1;
  $$.offset = $2;
 }
| LNAME '<' '>' offset '(' LSB ')'
 {
  $$ = nullgen;
  $$.type = D_STATIC;
  $$.sym = $1;
  $$.offset = $4;
 }
/*x: operand rules(x86) */
pointer:
  LSB
| LFP
| LSP { $$ = D_AUTO; }
/*x: operand rules(x86) */
rel:
 con '(' LPC ')'
 {
  $$ = nullgen;
  $$.type = D_BRANCH;
  $$.offset = $1 + pc;
 }
| LLAB offset
 {
  $$ = nullgen;
  $$.type = D_BRANCH;
  $$.sym = $1;
  $$.offset = $1->value + $2;
 }
| LNAME offset
 {
  $$ = nullgen;
  if(pass == 2)
   yyerror("undefined label: %s", $1->name);
  $$.type = D_BRANCH;
  $$.sym = $1;
  $$.offset = $2;
 }
/*x: operand rules(x86) */
offset:
 /* empty */ { $$ = 0; }
| '+' con    { $$ = $2; }
| '-' con    { $$ = -$2; }
/*x: operand rules(x86) */
rem:
  reg
| mem

rim:
  rem
| imm

rom:
  reg
| mem
| imm
| rel

| '*' reg  { $$ = $2; }
| '*' omem { $$ = $2; }
/*e: operand rules(x86) */
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
/*e: grammar(x86) */
/*e: 8a/a.y */
