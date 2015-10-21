(* TODO: remain object code gen chapter, debug chapter, and extensions *)

(* ------------------------------------------------------------------------- *)
(* Numbers and Strings *)
(* ------------------------------------------------------------------------- *)

(* line# *)
type pos = int 

(* enough for ARM 32 bits? on 64 bits machine it is enough :) *)
type integer = int 
(* virtual code address, increment by unit of 1 *)
type code_address = int
(* can be 0, negative, positive *)
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

type arith_operand =
  | Imm of integer (* characters are converted in integers *)
  | Reg of register
  | Shift of register * shift_reg_op * 
             (register, int (* between 0 and 31 *)) Common.either

  and shift_reg_op =
    | Sh_left | Sh_right
    | Sh_minus | Sh_at

type mov_operand = 
  | Ximm of ximm
  | Indirect of register * offset
  (* those below are all specialized forms of Indirect *)
  | Param of symbol option * offset (* FP *)
  | Local of symbol option * offset (* SP *)
  (* stricter: we disallow anonymous SB *)
  | Extern of extern_symbol (* SB *)


  and ximm =
    | Imm2 of integer
    | String of string (* limited to 8 characters *)
    (* Float? *)

    (* stricter: we disallow address of FP or SP *)
    | Address of extern_symbol

type branch_operand =
  (* nireg *)
  | SymbolJump of extern_symbol
  | IndirectJump of register

  (* rel *)
  (* before resolve *)
  | Relative of int (* PC *)
  | Label of label * offset (* useful to have offset? *)
  (* after resolve *)
  | Absolute of code_address
  

(* ------------------------------------------------------------------------- *)
(* Instructions *)
(* ------------------------------------------------------------------------- *)

type instr = 
  | NOP (* virtual, removed by linker *)

  | Arith of arith_opcode * arith_operand * register * register option

  | MOV of move_size * mov_operand * mov_operand (* virtual *)
  | SWAP of move_size (* actually only (Byte x) *) * 
       register (* indirect *) * register * register option

  | B of branch_operand
  | BL of branch_operand
  | RET (* virtual *)

  | Cmp of cmp_opcode * arith_operand * register
  (* TODO: normally just rel here, relative jump or label *)
  | Bxx of condition * branch_operand (* virtual, sugar *) 

  | SWI
  | RFE (* virtual, sugar for MOVM *)

  and arith_opcode = 
    (* logic *)
    | AND | ORR | EOR
    (* arith *)
    | ADD | SUB | MUL | DIV | MOD (* DIV and MOD are virtual *)
    | SLL | SRL | SRA (* virtual, sugar for bitshift register *)
    (* less useful *)
    | BIC  | ADC | SBC  | RSB | RSC
    (* middle operand always empty (could lift up and put special type) *)
    | MVN 
  and cmp_opcode = 
    | CMP
    (* less useful *)
    | TST | TEQ | CMN

  and condition =
    | EQ | NEQ
    | GT of sign | LT of sign | GE of sign | LE of sign
    (* ????? *)
    | MI | PL | VS | VC
    (* ????? AL | NV *)

   and sign = Signed | Unsigned
   and move_size = Byte of sign | Word | HalfWord of sign

type pseudo_instr =
  (* stricter: we allow only SB names (extern_symbol) *)
  | TEXT of extern_symbol (* offset is 0 *) * attributes * int
  | GLOBL of extern_symbol (* offset can be <> 0?? *) * attributes * int
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

(* after resolve there is no more L and branch operand has no more 
 * Relative or Label (converted in Absolute)
 *)
type program = (line * pos) list
