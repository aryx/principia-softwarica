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

%token <unit> TNOP
%token <Ast.arith_code> TARITH

%token <Ast.move_size> TMOV
%token <Ast.move_size> TSWAP

%token <unit> TB  TBL TRET
%token <Ast.cmp_opcode> TCMP   
%token <Ast.condition> TBxx TCOND

%token <unit> TSWI TRFE

%token <unit> TTEXT TGLOBL 
%token <unit> TDATA TWORD 

/*(*-----------------------------------------*)*/
/*(*2 registers *)*/
/*(*-----------------------------------------*)*/

%token <Ast.register> TRxx
%token <unit> TR

%token <unit> TPC TSB TFP TSP

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

%token <unit> TSEMICOLON TDOT TCOMMA TDOLLAR
%token <unit> TOPAR TCPAR
%token <unit> EOF

/*(*-----------------------------------------*)*/
/*(*2 Operators *)*/
/*(*-----------------------------------------*)*/

%token <unit> TPLUS TMINUS TTILDE TMUL TDIV TMOD

/*(*************************************************************************)*/
/*(*1 Priorities *)*/
/*(*************************************************************************)*/

/*(*************************************************************************)*/
/*(*1 Rules type declaration *)*/
/*(*************************************************************************)*/

%start prog
%type <Ast.program> program

%%

/*(*************************************************************************)*/
/*(*1 Program *)*/
/*(*************************************************************************)*/

program: lines EOF { List.rev $1 }

line: 
 |               TSEMICOLON { [] }
 | instr         TSEMICOLON { [I $1] }
 | pseudo_instr  TSEMICOLON { [P $1] }
 | TIDENT TCOLON line { $3 @ [L $1] }

/*(*************************************************************************)*/
/*(*1 Pseudo instruction *)*/
/*(*************************************************************************)*/
pseudo_instr:
 | TTEXT name attr_opt TCOMMA imm
 | TGLOBL name attr_opt TCOMMA imm
 | TDATA name TSLASH con TCOMMA ximm

attr_opt:
 | /* empty */ { [] }
 | TCOMMA con  { [$2] }


/*(*************************************************************************)*/
/*(*1 Instruction *)*/
/*(*************************************************************************)*/

instr:
 | TNOP { }

 | TARITH cond  imsr TCOMMA reg TCOMMA reg
 | TARITH cond  imsr TCOMMA reg 

 | TMOV   cond  gen  TCOMMA gen

 | TSWAP  cond  reg  TCOMMA ireg
 | TSWAP  cond  ireg TCOMMA reg
 | TSWAP  cond  reg  TCOMMA ireg TCOMMA reg

 | TB  cond branch
 | TBL cond branch
 | TCMP cond imsr TCOMMA reg
 | TBxx cond rel

 | TRET cond
 | TSWI cond 
 | TRFE cond

/*(*************************************************************************)*/
/*(*1 Operand *)*/
/*(*************************************************************************)*/

imsr:
 | imm   { $1 }
 | shift { $1 }
 | reg   { $1 }

imm: TDOLLAR con      { $1 }

con:
 | TINT { $1 }
 | TMINUS con { - $1 }
 | TPLUS  con { $1 }
 | TTILDE con { failwith "tilde??" }

reg:
 | TRxx                { $1 }
 | TR TOPar expr TCPar { R $2 }

shift:
 | reg TSHL rcon
 | reg TSHR rcon
 | reg TSHMINUS rcon
 | reg TSHAT rcon

rcon:
 | reg { Left $1 }
 | con { if $1 >= 0 && <= 31 
         then Right $1 
         else error "shift value out of range" 
       }

branch: 
 | rel 
 | name
 | ireg

rel:
 | TIDENT offset
 | con TOPAR TPC TCPAR

gen:
 | ximm
 | shift
 | reg

 | ioreg
 | name
 | con TOPAR pointer TCPAR

ximm:
 | TDOLLAR con { Imm $1 }
 | TDOLLAR TSTRING { String $2 }
 | TDOLLAR name

ioreg:
 | ireg
 | con TOPAR reg TCPAR

ireg: TOPAR reg TCPAR

name: TIDENT offset TOPAR pointer TCPAR

pointer: 
 | TSB
 | TSP
 | TFP

offset:
 | /* empty */ { 0 }
 | TPLUS  con  { $2 }
 | TMINUS con  { - $2 }


/*(*************************************************************************)*/
/*(*1 Other *)*/
/*(*************************************************************************)*/
 

/*(*************************************************************************)*/
/*(*1 EBNF *)*/
/*(*************************************************************************)*/

lines: 
| /*empty*/ { [] }
| lines line { $2 @ $1 }
