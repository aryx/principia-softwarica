
type object_code = 
 Ast_asm.program * Common.filename * (Ast_pos.pos * line_directive) list

(* can normalize before? *)
let save_obj prog file =
  Marshall.
