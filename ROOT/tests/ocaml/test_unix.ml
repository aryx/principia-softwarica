
let main () =
  (* but this actually redirects to sys_getenv which is builtin *)
  (*
  let s = Unix.getenv "HOME" in (* Not_found in Plan9 *)
  print_endline s;
  *)
  let xs = Unix.environment () in
  xs |> Array.iter print_endline

let _ = main ()
