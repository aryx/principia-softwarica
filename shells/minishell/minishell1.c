#include <u.h>
#include <libc.h>

void main() {
  char buf[256];
  char *args[2];
  int n;

  for(;;) {
    pwrite(1, "$ ", 2, 0LL);
    n = pread(0, buf, sizeof(buf)-1, 0LL);
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
  exits(0);
}
