
type object_code = 
 Ast_asm.program * Common.filename * (Ast_pos.pos * line_directive) list

val save: object_code -> Common.filename -> unit
