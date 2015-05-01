
let main () =
  Draw.initdraw None "test_draw_ml";

  Draw.set_color 10 10 10 255;
  Draw.line 10 10 100 100;
  Draw.string 200 200 "this is an ocaml test";

  Unix.sleep 5;
  ()

let _ = main ()
  
