
(* final program has no more label (Defs and Uses) or 
 * relative jump or labels in branching instructions.
 *)
val resolve: Ast.program -> Ast.program
