%{
open Ast

(*****************************************************************************)
(* Prelude *)
(*****************************************************************************)

let error s =
  failwith (spf "Syntax error: %s at line" s !Lexer.line)

%}

/*(*************************************************************************)*/
/*(*1 Tokens *)*/
/*(*************************************************************************)*/

/*(*-----------------------------------------*)*/
/*(*2 opcodes *)*/
/*(*-----------------------------------------*)*/

%token <Ast.arith_code> TARITH
%token <Ast.move_size> TMOV TSWAP
%token TB  TBL TRET
%token <Ast.cmp_opcode> TCMP   
%token <Ast.condition> TBxx TCOND
%token TSWI TRFE

%token TTEXT TGLOBL 
%token TDATA TWORD 

/*(*-----------------------------------------*)*/
/*(*2 registers *)*/
/*(*-----------------------------------------*)*/

%token <Ast.register> TRxx
%token TR
%token TPC TSB TFP TSP

/*(*-----------------------------------------*)*/
/*(*2 Constants *)*/
/*(*-----------------------------------------*)*/

%token <int> TINT
%token <float> TFLOAT
%token <string> TSTRING

/*(*-----------------------------------------*)*/
/*(*2 Names *)*/
/*(*-----------------------------------------*)*/
%token <string> TIDENT

/*(*-----------------------------------------*)*/
/*(*2 Punctuation *)*/
/*(*-----------------------------------------*)*/

%token TSEMICOLON TCOLON TDOT TCOMMA TDOLLAR
%token TOPAR TCPAR
%token EOF

/*(*-----------------------------------------*)*/
/*(*2 Operators *)*/
/*(*-----------------------------------------*)*/

%token TSHL TSHR   TSHMINUS TSHAT
%token TPLUS TMINUS TTILDE TMUL TMOD
%token TSLASH

/*(*************************************************************************)*/
/*(*1 Priorities *)*/
/*(*************************************************************************)*/
%left TOR
%left TXOR
%left TAND
%left TLT TGT
%left TPLUS TMINUS
%left TMUL TSLASH TMOD

/*(*************************************************************************)*/
/*(*1 Rules type declaration *)*/
/*(*************************************************************************)*/

%type <Ast_asm.program> program
%start program

%%

/*(*************************************************************************)*/
/*(*1 Program *)*/
/*(*************************************************************************)*/

program: lines EOF { List.rev $1 }

lines: 
 | /*empty*/  { [] }
 | lines line { $2 @ $1 }

line: 
 |               TSEMICOLON { [] }
 | instr         TSEMICOLON { [I $1] }
 | pseudo_instr  TSEMICOLON { [P $1] }
 | TIDENT TCOLON line { $3 @ [L $1] }

/*(*************************************************************************)*/
/*(*1 Pseudo instructions *)*/
/*(*************************************************************************)*/
/*(* can't factorize in attr_opt, shift/reduce conflict with TCOMMA *)*/
pseudo_instr:
 | TTEXT  name TCOMMA imm    { }
 | TGLOBL name TCOMMA imm    { }

 /*(* todo: would be better to have mnemonics for that too *)*/
 | TTEXT  name TCOMMA con TCOMMA imm    { }
   /*{ match $2 with 0 -> [] | 1 -> NOPROF | 2 -> DUPOK }*/
 | TGLOBL name TCOMMA con TCOMMA imm    { }
   /*{ match $2 with 0 -> [] | 1 -> NOPROF | 2 -> DUPOK }*/
 | TDATA name TSLASH con TCOMMA ximm  { }


/*(*************************************************************************)*/
/*(*1 Instructions *)*/
/*(*************************************************************************)*/

instr:
 | TARITH cond  imsr TCOMMA reg TCOMMA reg { }
 | TARITH cond  imsr TCOMMA reg            { }

 | TMOV   cond  gen  TCOMMA gen  { }

 | TSWAP  cond  reg  TCOMMA ireg { }
 | TSWAP  cond  ireg TCOMMA reg  { }
 | TSWAP  cond  reg  TCOMMA ireg TCOMMA reg { }

 | TB  cond branch { }
 | TBL cond branch { }
 | TCMP cond imsr TCOMMA reg { } 
 | TBxx cond rel { }
 | TRET cond { }

 | TSWI cond imm { }
 | TRFE cond { }

/*(*************************************************************************)*/
/*(*1 Operands *)*/
/*(*************************************************************************)*/

imsr:
 | imm   { Imm $1 }
 | shift { $1 }
 | reg   { Reg $1 }


imm: TDOLLAR con      { $2 }

con:
 | TINT { $1 }
 | TMINUS con { - $2 }
 | TPLUS  con { $2 }
 | TTILDE con { failwith "TODO: tilde??" }
 | TOPAR expr TCPAR { $2 }

expr:
 | con { $1 }

 | expr TPLUS expr  { $1 + $3 }
 | expr TMINUS expr { $1 - $3 }
 | expr TMUL expr   { $1 * $3 }
 | expr TSLASH expr   { $1 / $3 }
 | expr TMOD expr   { $1 % $3 }

 | expr TLT TLT expr { $1 << $4 }
 | expr TGT TGT expr { $1 >> $4 }

 | expr TAND expr    { $1 & $3 }
 | expr TOR expr     { $1 | $3 }
 | expr TXOR expr    { $1 ^ $3 }


reg:
 | TRxx                { $1 }
 | TR TOPAR expr TCPAR { R $3 }


shift:
 | reg TSHL rcon     { }
 | reg TSHR rcon     { }
 | reg TSHMINUS rcon { }
 | reg TSHAT rcon    { }

rcon:
 | reg { Left $1 }
 | con { if $1 >= 0 && <= 31 
         then Right $1 
         else error "shift value out of range" 
       }



gen:
 | ximm  { $1 }
 | shift { $1 }
 | reg   { $1 }

 | ioreg { $1 }
 | name  { $1 }
 | con TOPAR pointer TCPAR { failwith "TODO" }

ximm:
 | TDOLLAR con     { Imm $2 }
 | TDOLLAR TSTRING { String $2 }
 | TDOLLAR name    { failwith "TODO" }

ioreg:
 | ireg                { Indirect ($1, 0) }
 | con TOPAR reg TCPAR { Indirect ($3, $1) }

ireg: TOPAR reg TCPAR { $2 }

name: TIDENT offset TOPAR pointer TCPAR { }

pointer: 
 | TSB  { }
 | TSP  { }
 | TFP  { }

offset:
 | /* empty */ { 0 }
 | TPLUS  con  { $2 }
 | TMINUS con  { - $2 }



branch: 
 | rel   { $1 }
 | name  { (* only SB? *) }
 | ireg  { }

rel:
 | TIDENT offset        { Label ($1, $2) }
 | con TOPAR TPC TCPAR  { Relative $1 }


/*(*************************************************************************)*/
/*(*1 Misc *)*/
/*(*************************************************************************)*/

cond:
 | /* empty */ { }
 | cond TCOND  { }
