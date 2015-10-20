
(* final program has no more label (Defs and Uses) or 
 * relative jump in branching instructions.
 *)
val resolve: Ast.program -> Ast.program
