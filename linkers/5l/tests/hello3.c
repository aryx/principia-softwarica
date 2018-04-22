#include <u.h>
#include <libc.h>

extern char* hello;

int main() {
  fprint(1, hello);
  exits(0);
}
