
#include <draw.h>

void caml_draw_initdraw(long x, long y) {
  USED(x); USED(y);
  initdraw(nil, nil, "test_draw");
}
