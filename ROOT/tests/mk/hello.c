//#include <u.h>
//#include <libc.h>
extern  int     fprint(int, char*, ...);
extern  void    exits(char*);

extern char* hello;

int main() {
  fprint(1, hello);
  exits(0);
}
