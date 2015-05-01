
(* may raise exn? *)
external initdraw : string option -> string -> unit = "caml_draw_initdraw"

external set_color : int -> int -> int -> int -> unit = "caml_draw_set_color"
external line: int -> int -> int -> int -> unit = "caml_draw_line"
external string: int -> int -> string -> unit = "caml_draw_string"

