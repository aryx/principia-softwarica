open Common

let thechar = '5'
let thestring = "arm"

let assemble infile outfile =
  let prog = Parse.parse infile in
  (* TODO: solve labels *)
  Gen.save_obj prog outfile

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

  if !outfile = ""
  then begin 
    let b = Filename.basename !infile in
    if b =~ "\\(.*\\)\\.s"
    then outfile := Common.matched1 b ^ (spf ".%c" thechar)
    else outfile := b ^ (spf ".%c" thechar)
  end;
  assemble !infile !outfile
  

let _ = main ()
