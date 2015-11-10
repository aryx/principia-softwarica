open Common

(*****************************************************************************)
(* Prelude *)
(*****************************************************************************)
(* Limitations compared to 5a:
 *  - no parallel multi files thing (not the place anyway)
 *)

let thechar = '5'

let assemble infile outfile =
  let prog = Parse_asm.parse infile in
  let prog = Resolve_labels.resolve prog in
  Object_code.save_obj prog outfile

let main () =
  let infile = ref "" in
  let outfile = ref "" in
  Arg.parse [
    "-o", Arg.Set_string outfile,
    " <file> output file";
  ] 
  (fun f -> 
    if !infile <> ""
    then failwith "already specified an input file";
    infile := f;
  )
  (spf "%ca [-options] file.s" thechar);

  if !infile = ""
  then failwith "we need an input file";
  if !outfile = ""
  then begin 
    let b = Filename.basename !infile in
    if b =~ "\\(.*\\)\\.s"
    then outfile := Common.matched1 b ^ (spf ".%c" thechar)
    else outfile := b ^ (spf ".%c" thechar)
  end;
  assemble !infile !outfile
  

let _ = 
  main ()
