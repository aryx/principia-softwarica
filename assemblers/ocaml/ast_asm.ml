(* TODO: remain debugging chapter, and extensions *)

(* ------------------------------------------------------------------------- *)
(* Numbers and Strings *)
(* ------------------------------------------------------------------------- *)

(* line# *)
type pos = int 

(* enough for ARM 32 bits? on 64 bits machine it is enough :) *)
type integer = int 
(* increments by unit of 1 *)
type virtual_code_address = int
(* can be 0, negative, positive *)
type offset = int

type label = string
type symbol = string
type entity = symbol * bool (* static *)

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
  | Imsr of arith_operand
  | Ximm of ximm
  | Indirect of register * offset
  (* those below are all specialized forms of Indirect *)
  | Param of symbol option * offset (* FP *)
  | Local of symbol option * offset (* SP *)
  (* stricter: we disallow anonymous offsets to SB *)
  | Entity of entity * offset (* SB *) 

  and ximm =
    | String of string (* limited to 8 characters *)
    (* Float? *)
    (* stricter: we disallow address of FP or SP, and offset to SB *)
    | Address of entity

type branch_operand =
  (* nireg *)
  | SymbolJump of entity * offset
  | IndirectJump of register

  (* rel *)
  (* before resolve *)
  | Relative of int (* relative to PC, in units of virtual_code_address *)
  | Label of label * offset (* useful to have offset? *)
  (* after resolve *)
  | Absolute of virtual_code_address
  

(* ------------------------------------------------------------------------- *)
(* Instructions *)
(* ------------------------------------------------------------------------- *)

type instr = 
  | NOP (* virtual, removed by linker *)

  (* Arithmetic *)
  | Arith of arith_opcode * arith_operand * register * register option

  (* Memory *)
  | MOV of move_size * mov_operand * mov_operand (* virtual *)
  | SWAP of move_size (* actually only (Byte x) *) * 
       register (* indirect *) * register * register option

  (* Control flow *)
  | B of branch_operand
  | BL of branch_operand
  | RET (* virtual *)
  | Cmp of cmp_opcode * arith_operand * register
  (* TODO: normally just rel here, relative jump or label *)
  | Bxx of condition * branch_operand (* virtual, sugar *) 

  (* System *)
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
  (* stricter: we allow only SB for TEXT and GLOBL, and no offset *)
  | TEXT of symbol * attributes * int
  | GLOBL of symbol (* can have offset? *) * attributes * int
  | DATA of symbol * offset * int (* size *) * ximm
  (* any ximm? even String? And Float? *)
  | WORD of (int, ximm) Common.either

  and attributes = attribute list
  and attribute = DUPOK | NOPROF

(* ------------------------------------------------------------------------- *)
(* Program *)
(* ------------------------------------------------------------------------- *)

type line = 
  | P of pseudo_instr
  | I of instr * condition option (* * bitset list *)
  | L of label
  (* D of line_directive? use Lexer_asm.line_directives instead *)

type program = (line * pos) list
