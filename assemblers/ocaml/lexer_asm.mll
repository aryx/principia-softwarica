{
open Common
open Ast
open Parse

(*****************************************************************************)
(* Prelude *)
(*****************************************************************************)
(* Limitations compared to 5a:
 *  - does not handle unicode
 *  - does not recognize the uU lL suffix 
%     (but was skipped by 5a anyway)
 *  - does not handle preprocessing directives, assume external cpp
 *    (but better to factorize code and separate concerns)
 *)

let line = ref 1

let error s =
  failwith (spf "Lexical error: %s (line %d)" s !line)

(* stricter: we disallow \ with unknown character *)
let code_of_escape_char c =
  match c with
  | 'n' -> Char.code '\n' | 'r' -> Char.code '\r' 
  | 't' -> Char.code '\t' | 'b' -> Char.code '\b' 
  | 'f' -> error "unknown \\f"
  (* could be removed, special 5a escape char *)
  | 'a' -> 0x07 | 'v' -> 0x0b | 'z' -> 0x00
  | _ -> error "unknown escape sequence"

let string_of_ascii i =
  String.make 1 (Char.chr i)

}

(*****************************************************************************)
(* Regexps aliases *)
(*****************************************************************************)
let letter = ['a'-'z''A'-'Z']
let space = [' ''\t']

let digit = ['0'-'9']
let hex = (digit | ['A'-'F''a'-'f'])
let oct = ['0'-'7']

(*****************************************************************************)
(* Main rule *)
(*****************************************************************************)
rule token = parse

  (* ----------------------------------------------------------------------- *)
  (* Spacing/comments *)
  (* ----------------------------------------------------------------------- *)
  | space+ { token lexbuf }

  | "//" [^'\n']* { token lexbuf }
  | "/*"          { comment lexbuf }

  | '\n' { incr line; TSEMICOLON }

  (* ----------------------------------------------------------------------- *)
  (* Symbols *)
  (* ----------------------------------------------------------------------- *)
  | ';' { TSEMICOLON }
  | ',' { TCOMMA }
  | '(' { TOPAR } | ')' { TCPAR }
  | '$' { TDOLLAR }

  | '+' { TPLUS } | '-' { TMINUS } | '*' { TMUL }
  (* used for division and for DATA *)
  | '/' { TSLASH } 
  | '%' { TMOD }

  (* has to be before the rule for identifiers *)
  | '.' { TDOT }

  (* ----------------------------------------------------------------------- *)
  (* Mnemonics and identifiers *)
  (* ----------------------------------------------------------------------- *)
  | "R" (digit+ as s) 
      { let i = int_of_string s in 
        if i <= 15 && i >=0
        then TRxx i
        else error ("register number not valid")
      }

  (* actually for '.' 5a imposes to have an isalpha() after *)    
  | (letter | '_' | '@' | '.') (letter | digit | '_' | '$' )* {
      let s = Lexing.lexeme lexbuf in
      match x with
      (* instructions *)
      | "NOP" -> TNOP

      | "AND" -> TARITH AND | "ORR" -> TARITH ORR | "EOR" -> TARITH EOR

      | "ADD" -> TARITH ADD | "SUB" -> TARITH SUB
      | "MUL" -> TARITH MUL | "DIV" -> TARITH DIV | "MOD" -> TARITH MOD
      | "SLL" -> TARITH SLL | "SRL" -> TARITH SRL | "SRA" -> TARITH SRA

      | "BIC" -> TARITH BIC
      | "ADC" -> TARITH ADC | "SBC" -> TARITH SBC
      | "RSB" -> TARITH RSB | "RSC" -> TARITH RSC

      | "MOVW" -> TMOV Word
      | "MOVB" -> TMOV (Byte Signed) | "MOVBU" -> TMOV (Byte Unsigned)
      | "MOVH" -> TMOV (HalfWord Signed) | "MOVHU" -> TMOV (HalfWord Unsigned)

      | "B" -> TB | "BL" -> TBL
      | "CMP" -> TCMP CMP 
      | "TST" -> TCMP TST | "TEQ" -> TCMP TEQ | "CMN" -> TCMP CMN
      | "RET" -> TRET
      | "BEQ" -> Bxx EQ | "BNE" -> Bxx NE
      | "BGT" -> Bxx GT | "BLT" -> Bxx LT | "BGE" -> Bxx GE | "BLE" -> Bxx LE
      | "BHI" -> Bxx HI | "BLO" -> Bxx LO | "BHS" -> Bxx HS | "BLS" -> Bxx LS
      | "BMI" -> Bxx MI | "BPL" -> Bxx PL | "BVS" -> Bxx VS | "BVC" -> Bxx VC

      | "SWI" -> TSWI
      | "RFE" -> TRFE

      (* pseudo *)
      | "TEXT" -> TTEXT | "GLOBL" -> TGLOBL
      | "WORD" -> TWORD | "DATA" -> TDATA

      (* registers (see also the special rule above for R digit+) *)
      | "R" -> TR

      (* pseudo registers *)
      | "PC" -> TPC | "SB" -> TSB | "SP" -> TSP | "FP" -> TFP

      (* condition *)

      | ".EQ" -> TCOND EQ | ".NE" -> TCOND NE
      | ".GT" -> TCOND GT | ".LT" -> TCOND LT 
      | ".GE" -> TCOND GE | ".LE" -> TCOND LE
      | ".HI" -> TCOND HI | ".LO" -> TCOND LO 
      | ".HS" -> TCOND HS | ".LS" -> TCOND LS
      | ".MI" -> TCOND MI | ".PL" -> TCOND PL 
      | ".VS" -> TCOND VS | ".VC" -> TCOND VC

      (* less: could impose is_lowercase? *)
      | _ -> TIDENT x
    }

  (* ----------------------------------------------------------------------- *)
  (* Numbers *)
  (* ----------------------------------------------------------------------- *)
  | "0"  (oct+ as s) { TINT (int_of_string ("0o" ^ s))  }
  | "0x" hex+        { TINT (int_of_string (Lexing.lexeme lexbuf)) }
  | digit+           { TINT (int_of_string (Lexing.lexeme lexbuf)) }

  (* I impose some digit+ after '.' and after 'e' *)
  | (digit+ | digit* '.' digit+) (['e''E'] ('+' | '-')? digit+)?
     { TFLOAT (float_of_string (Lexing.lexeme lexbuf)) }

  (* ----------------------------------------------------------------------- *)
  (* Chars/Strings *)
  (* ----------------------------------------------------------------------- *)
  | "'" { TINT (char lexbuf) }
  | '"' { 
    let s = string lexbuf in
    (* TODO? why this limit though? *)
    if String.length > 8
    then error ("string constant too long")
    else TSTRING s
  }

  (* ----------------------------------------------------------------------- *)
  (* Misc *)
  (* ----------------------------------------------------------------------- *)
  (*| "#line" ... *)

  (* ----------------------------------------------------------------------- *)
  | eof { EOF }
  | _ { error "unrecognized character" }

(*****************************************************************************)
(* Rule char *)
(*****************************************************************************)
and char = parse
  | "''" { Char.code '\'' }
  | "\\" ((oct oct? oct?) as s) "'" { int_of_string ("0o" ^ s) }
  | "\\" (['a'-'z'] as c) "'"   { code_of_escape_char c }
  | [^ '\\' '\'' '\n'] as c  "'"     { Char.code c }
  | '\n' { error "newline in character" }
  | eof  { error "end of file in character" }
  | _    { error "missing '" }

(*****************************************************************************)
(* Rule string *)
(*****************************************************************************)
and string = parse
  | '"' { "" }
  | "\\" ((oct oct oct) as s)
      { let i = int_of_string ("0o" ^ s) in string_of_ascii i ^ string lexbuf }
  | "\\" (['a'-'z'] as c) 
      { let i = code_of_escape_char c in string_of_ascii i ^ string lexbuf  }
  | [^ '\\' '"' '\n']+   
      { let x = Lexing.lexeme lexbuf in x ^ string lexbuf }
  | '\n' { error "newline in string" }
  | eof  { error "end of file in string" }
  | _    { error "undefined character in string" }


(*****************************************************************************)
(* Rule comment *)
(*****************************************************************************)
and comment = parse
  | "*/"          { token lexbuf }
  | [^ '*' '\n']+ { comment lexbuf }
  | '*'           { comment lexbuf }
  | '\n'          { incr line; comment lexbuf }
  | eof           { error "end of file in comment" }
