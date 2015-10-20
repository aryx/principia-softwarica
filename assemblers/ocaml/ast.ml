(* TODO: remain object code gen chapter, debug chapter, and extensions
 *)

(* ------------------------------------------------------------------------- *)
(* Names and values *)
(* ------------------------------------------------------------------------- *)

type pos = int (* line# *)

(* enough? on 64 bits machine it is enough :) *)
type integer = int 
(* virtual code address, increments by unit of 1 *)
type code_address = int
type offset = int

type label = string
type symbol = string
type extern_symbol = symbol * bool (* static *) * offset

(* ------------------------------------------------------------------------- *)
(* Operands *)
(* ------------------------------------------------------------------------- *)

type register = R of int (* between 0 and 15 *)
let rSB = R 12
let rSP = R 13
let rLINK = R 14
let rPC = R 15

type imsr =
  | Imm of integer (* characters are converted in integers *)
  | Reg of register
  | Shift of register * shift_reg_op * (register, int) Common.either

  and shift_reg_op =
    | Sh_left | Sh_right
    | Sh_minus | Sh_at

type gen = 
  | Ximm of ximm

  | Indirect of register * offset
  | Param of symbol option * offset (* FP *)
  | Local of symbol option * offset (* SP *)
  (* stricter: disallow anonymous SB *)
  | Extern of extern_symbol (* SB *)


  and ximm =
    | Imm2 of integer
    | String of string (* limited to 8 characters *)
    (* Float? *)

    (* stricter: disallow address of FP or SP *)
    | Address of extern_symbol

type branch =
  | Symbol of extern_symbol
  | IndirectJump of register

  (* before resolve *)
  | Relative of int (* PC *)
  | Label of label * offset 
  (* after resolve *)
  | Absolute of code_address
  

(* ------------------------------------------------------------------------- *)
(* Instructions *)
(* ------------------------------------------------------------------------- *)

type instr = 
  | NOP (* virtual *)

  | Arith of arith_opcode * imsr * register * register option

  | MOV of move_size * gen * gen (* virtual *)
  | SWAP of move_size (* actually only Byte *) * 
       register (* indirect *) * register * register option

  | B of branch
  | BL of branch
  | RET (* virtual *)

  | Cmp of cmp_opcode * imsr * register
  | Bxx of condition * branch (* virtual *) (* TODO: normally just rel here *)

  | SWI
  | RFE

  and arith_opcode = 
    | ADD | SUB | MUL 
    | DIV | MOD (* virtual *)
    | AND | ORR | EOR
    | SLL | SRL | SRA
    (* less useful *)
    | BIC  | ADC | SBC  | RSB | RSC
    | MVN (* middle operand always empty (could lift up and put special type) *)
  and cmp_opcode = 
    | CMP
    | TST | TEQ | CMN

  and condition =
    | EQ | NEQ
    | GT of sign | LT of sign | GE of sign | LE of sign
    (* was | HI | LO | HS | LS *)
    (* ????? *)
    | MI | PL | VS | VC
    (* ????? AL | NV *)

   and sign = Signed | Unsigned
   and move_size = Byte of sign | Word | HalfWord of sign


(* stricter: allow only SB name *)
type pseudo_instr =
  | TEXT of extern_symbol (* offset is 0 *) * attributes * int
  | GLOBL of extern_symbol * attributes * int
  | DATA of extern_symbol * int (* size *) * ximm
  | WORD of ximm
  and attributes = attribute list
  and attribute = DUPOK | NOPROF

(* ------------------------------------------------------------------------- *)
(* Program *)
(* ------------------------------------------------------------------------- *)

type line = 
  | P of pseudo_instr
  | I of instr * condition option (* * bitset list *)
  | L of label

type program = (line * pos) list

