#include <u.h>
#include <libc.h>

//void bar(int x, int y) {
//}
//
//void foo(int x, int y) {
//  pwrite(1, "hello world\n", 12, 0);
//  bar(x, y);
//}
//
//
////@Scheck: not dead, entry point!
//void main() {
//  //print("hello world\n");
//  foo(1, 2);
//}

char *hello = "hello world\n";

void main() 
{
  fprint(1, hello);
  exits(0);
}
