/*s: rc/syn.y */
%{
#include "rc.h"
#include "fns.h"
%}

%union {
 struct Tree *tree;
};

/*s: token declarations */
%token <tree> FOR IN WHILE IF NOT SWITCH FN
%token <tree> WORD
%token <tree> TWIDDLE BANG SUBSHELL 
%token <tree> REDIR DUP PIPE
%token ANDAND OROR
%token COUNT SUB
%token SIMPLE ARGLIST WORDS BRACE PAREN PCMD PIPEFD /* not used in syntax */
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
/*s: type declarations */
%type<tree> line paren brace body cmdsa cmdsan assign epilog redir
%type<tree> cmd simple first word comword keyword words
/*e: type declarations */

%%
/*s: grammar */
rc: 
    /*empty*/       { return 1;}
|   line '\n'       { return !compile($1);}

line:   
    cmd
|   cmdsa line      {$$=tree2(';', $1, $2);}

cmdsa:  
    cmd ';'
|   cmd '&'         {$$=tree1('&', $1);}


cmd: 
    /*empty*/           {$$=nil;}
|   simple              {$$=simplemung($1);}
|   brace epilog        {$$=epimung($1, $2);}

|   cmd ANDAND cmd      {$$=tree2(ANDAND, $1, $3);}
|   cmd OROR cmd        {$$=tree2(OROR, $1, $3);}
|   cmd PIPE cmd        {$$=mung2($2, $1, $3);}

|   IF paren {skipnl();} cmd  {$$=mung2($1, $2, $4);}
|   IF NOT   {skipnl();} cmd  {$$=mung1($2, $4);}

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

|   WHILE paren {skipnl();} cmd    {$$=mung2($1, $2, $4);}
|   SWITCH word {skipnl();} brace  {$$=tree2(SWITCH, $2, $4);}

|   TWIDDLE word words  {$$=mung2($1, $2, $3);}

|   redir cmd  %prec BANG   {$$=mung2($1, $1->child[0], $2);}
|   assign cmd %prec BANG   {$$=mung3($1, $1->child[0], $1->child[1], $2);}

|   BANG cmd            {$$=mung1($1, $2);}
|   SUBSHELL cmd        {$$=mung1($1, $2);}

|   FN words brace  {$$=tree2(FN, $2, $3);}
|   FN words        {$$=tree1(FN, $2);}


brace:  '{' body '}'        {$$=tree1(BRACE, $2);}
paren:  '(' body ')'        {$$=tree1(PCMD, $2);}

body:   
    cmd
|   cmdsan body     {$$=tree2(';', $1, $2);}

cmdsan: 
    cmdsa
|   cmd '\n'

epilog: 
    /*empty*/           {$$=nil;}
|   redir epilog        {$$=mung2($1, $1->child[0], $2);}


assign: first '=' word      {$$=tree2('=', $1, $3);}


redir:  
    REDIR word      {$$=mung1($1, $1->rtype==HERE ? heredoc($2) : $2);}
|   DUP


simple:
    first
|   simple word         {$$=tree2(ARGLIST, $1, $2);}
|   simple redir        {$$=tree2(ARGLIST, $1, $2);}

/* diff with word? first cannot be a keyword */
first:  
    comword 
|   first '^' word      {$$=tree2('^', $1, $3);}

word:   
    comword
|   word '^' word       {$$=tree2('^', $1, $3);}
|   keyword             {lastword=true; $1->type=WORD;}

comword: 
    WORD
|   '$' word       {$$=tree1('$', $2);}
|   '$' word SUB words ')'  {$$=tree2(SUB, $2, $4);}
|   COUNT word      {$$=tree1(COUNT, $2);}
|   '"' word        {$$=tree1('"', $2);}
|   '`' brace       {$$=tree1('`', $2);}
|   '(' words ')'   {$$=tree1(PAREN, $2);}
|   REDIR brace     {$$=mung1($1, $2); $$->type=PIPEFD;}

keyword: FOR|IN|WHILE|IF|NOT|TWIDDLE|BANG|SUBSHELL|SWITCH|FN

words: 
    /*empty*/       {$$=(struct Tree*)nil;}
|   words word      {$$=tree2(WORDS, $1, $2);}
/*e: grammar */
/*e: rc/syn.y */
