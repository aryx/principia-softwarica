/*s: rc/syn.y */
%{
#include "rc.h"
#include "fns.h"
%}

%union {
 struct Tree *tree;
};

/*s: token declarations */
%token FOR IN WHILE IF NOT SWITCH FN
%token TWIDDLE BANG  /** ~ ! */
%token REDIR PIPE /** {>, <, <<, >>} | */
%token ANDAND OROR /** && || */
%token COUNT SUB /** $# ( */
%token WORD /** anything else (e.g. foo, --help, 42, /a/b/c, etc) */
/*x: token declarations */
/* not used in syntax */
%token SIMPLE 
%token ARGLIST WORDS
%token BRACE PAREN 
/*x: token declarations */
%token PCMD
/*x: token declarations */
%token SUBSHELL /** @ */
/*x: token declarations */
%token DUP
/*x: token declarations */
%token PIPEFD 
/*e: token declarations */
/*s: priority and associativity declarations */
/* operator priorities -- lowest first */
%left IF WHILE FOR SWITCH ')' NOT
%left ANDAND OROR
%left BANG SUBSHELL
%left PIPE
%left '^'
%right '$' COUNT '"'
%left SUB
/*e: priority and associativity declarations */
/*s: type [[declarations]] */
%type<tree> line cmd simple word comword
/*x: type [[declarations]] */
%type<tree> first keyword words
%type<tree> paren brace body cmdsa cmdsan assign epilog redir
%type<tree> FOR IN WHILE IF NOT SWITCH FN
%type<tree> TWIDDLE BANG SUBSHELL  REDIR DUP PIPE    WORD
/*e: type [[declarations]] */

%%
/*s: grammar */
rc: 
    /*empty*/       { return ERROR_1;}
|   line '\n'       { return !compile($1);}

/*s: line rule */
line:   
    cmd
/*s: line rule other cases */
|   cmdsa line      {$$=tree2(';', $1, $2);}
/*e: line rule other cases */
/*e: line rule */
/*s: cmd rule */
cmd: 
    /*empty*/           {$$=nil;}
|   simple              {$$=simplemung($1);}
/*s: cmd rule other cases */
|   cmd ANDAND cmd      {$$=tree2(ANDAND, $1, $3);}
|   cmd OROR cmd        {$$=tree2(OROR, $1, $3);}
/*x: cmd rule other cases */
|   BANG cmd            {$$=mung1($1, $2);}
/*x: cmd rule other cases */
|   TWIDDLE word words  {$$=mung2($1, $2, $3);}
/*x: cmd rule other cases */
|   cmd PIPE cmd        {$$=mung2($2, $1, $3);}
/*x: cmd rule other cases */
|   brace epilog        {$$=epimung($1, $2);}
/*x: cmd rule other cases */
|   redir cmd  %prec BANG
        {$$=mung2($1, $1->child[0], $2);}
/*x: cmd rule other cases */
|   IF paren {skipnl();} cmd  {$$=mung2($1, $2, $4);}
|   IF NOT   {skipnl();} cmd  {$$=mung1($2, $4);}

|   WHILE paren {skipnl();} cmd    {$$=mung2($1, $2, $4);}
|   SWITCH word {skipnl();} brace  {$$=tree2(SWITCH, $2, $4);}

 /*
  * if ``words'' is nil, we need a tree element to distinguish between 
  * for(i in ) and for(i), the former being a loop over the empty set
  * and the latter being the implicit argument loop.  so if $5 is nil
  * (the empty set), we represent it as "()".  don't parenthesize non-nil
  * functions, to avoid growing parentheses every time we reread the
  * definition.
  */
|   FOR '(' word IN words ')' {skipnl();} cmd
    {$$=mung3($1, $3,    $5 ? $5 : tree1(PAREN, $5), $8);}

|   FOR '(' word ')' {skipnl();} cmd
    {$$=mung3($1, $3, (struct Tree *)0, $6);}
/*x: cmd rule other cases */
|   FN words brace  {$$=tree2(FN, $2, $3);}
|   FN words        {$$=tree1(FN, $2);}
/*x: cmd rule other cases */
|   assign cmd %prec BANG   
      {$$=mung3($1, $1->child[0], $1->child[1], $2);}
/*x: cmd rule other cases */
|   SUBSHELL cmd        {$$=mung1($1, $2);}
/*e: cmd rule other cases */
/*e: cmd rule */
/*s: simple rule */
simple:
    first
|   simple word         {$$=tree2(ARGLIST, $1, $2);}
/*s: simple rule other cases */
|   simple redir        {$$=tree2(ARGLIST, $1, $2);}
/*e: simple rule other cases */
/*e: simple rule */

/*s: word rule */
word:   
    comword
|   word '^' word       {$$=tree2('^', $1, $3);}
|   keyword             {lastword=true; $1->type=WORD;}
/*e: word rule */
/*s: comword rule */
comword: 
    WORD
/*s: comword rule other cases */
|   '$' word        {$$=tree1('$', $2);}
|   COUNT word      {$$=tree1(COUNT, $2);}
|   '$' word SUB words ')'  {$$=tree2(SUB, $2, $4);}
/*x: comword rule other cases */
|   '(' words ')'   {$$=tree1(PAREN, $2);}
/*x: comword rule other cases */
|   REDIR brace     {$$=mung1($1, $2); $$->type=PIPEFD;}
/*x: comword rule other cases */
|   '`' brace       {$$=tree1('`', $2);}
/*x: comword rule other cases */
|   '"' word        {$$=tree1('"', $2);}
/*e: comword rule other cases */
/*e: comword rule */

/*s: other rules */
first:  
    comword 
|   first '^' word      {$$=tree2('^', $1, $3);}
/*x: other rules */
keyword: FOR|IN|WHILE|IF|NOT|TWIDDLE|BANG|SUBSHELL|SWITCH|FN
/*x: other rules */
words: 
    /*empty*/       {$$=(struct Tree*)nil;}
|   words word      {$$=tree2(WORDS, $1, $2);}
/*x: other rules */
cmdsa:  
    cmd ';'
|   cmd '&'         {$$=tree1('&', $1);}
/*x: other rules */
redir:  
    REDIR word      {$$=mung1($1, $1->rtype==HERE ? heredoc($2) : $2);}
/*s: redir rule other cases */
|   DUP
/*e: redir rule other cases */
/*x: other rules */
epilog: 
    /*empty*/           {$$=nil;}
|   redir epilog        {$$=mung2($1, $1->child[0], $2);}
/*x: other rules */
paren:  '(' body ')'        {$$=tree1(PCMD, $2);}
/*x: other rules */
brace:  '{' body '}'        {$$=tree1(BRACE, $2);}
/*x: other rules */
body:   
    cmd
|   cmdsan body     {$$=tree2(';', $1, $2);}

cmdsan: 
    cmdsa
|   cmd '\n'
/*x: other rules */
assign: first '=' word      {$$=tree2('=', $1, $3);}
/*e: other rules */
/*e: grammar */
/*e: rc/syn.y */
