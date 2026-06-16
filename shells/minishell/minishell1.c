/*s: minishell1.c */
#include <u.h>
#include <libc.h>

void main() {
  char buf[256];
  char *args[2];
  int n;

  for(;;) {
    write(STDOUT, "$ ", 2);
    n = read(STDIN, buf, sizeof(buf)-1);
    if(n <= 0)
      break;
    buf[n-1] = '\0';  /* strip newline */
    args[0] = buf;
    args[1] = nil;
    if(rfork(RFPROC|RFFDG) == 0) {
      exec(buf, args);
      exits("exec failed");
    }
    await(buf, sizeof(buf));
  }
  exits(nil);
}
/*e: minishell1.c */
