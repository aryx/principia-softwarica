
(* final program has no more label (defs and uses) or 
 * relative jump or labels in branching instructions.
 * Those are converted in absolute jumps.
 *)
val resolve: Ast.program -> Ast.program
