#include <u.h>
#include <libc.h>

void main() {
  //alt: print("hello world\n");
  //alt: fprint(1, "hello world\n");
  pwrite(1, "hello world\n", 12, 0);
}
