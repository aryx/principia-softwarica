#include <u.h>
#include <libc.h>

//@Scheck: not dead, entry point!
void main() {
  //print("hello world\n");
  pwrite(1, "hello world\n", 12, 0);
}
