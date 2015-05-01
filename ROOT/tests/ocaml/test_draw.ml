
let main () =
  Draw.initdraw None "test_draw_ml";

  Draw.set_color 10 10 10 255;
  Draw.line 10 10 100 100;
  

  Unix.sleep 5;
  ()

let _ = main ()
  
