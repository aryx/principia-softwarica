#include <u.h>
#include <libc.h>

void runcmd(char *cmd) {
  char *args[2];
  args[0] = cmd;
  args[1] = nil;
  exec(cmd, args);
  exits("exec failed");
}

void main() {
  char buf[256];
  char *p;
  int n, fd, pfd[2];

  for(;;) {
    pwrite(1, "$ ", 2, 0LL);
    n = pread(0, buf, sizeof(buf)-1, 0LL);
    if(n <= 0)
      break;
    buf[n-1] = '\0';

    if((p = strchr(buf, '>')) != nil) {
      *p++ = '\0';
      while(*p == ' ') p++;
      if(rfork(RFPROC|RFFDG) == 0) {
        fd = create(p, OWRITE, 0666);
        dup(fd, 1);
        close(fd);
        runcmd(buf);
      }
      await(buf, sizeof(buf));
    } else if((p = strchr(buf, '|')) != nil) {
      *p++ = '\0';
      while(*p == ' ') p++;
      pipe(pfd);
      if(rfork(RFPROC|RFFDG) == 0) {
        close(pfd[0]);
        dup(pfd[1], 1);
        close(pfd[1]);
        runcmd(buf);
      }
      if(rfork(RFPROC|RFFDG) == 0) {
        close(pfd[1]);
        dup(pfd[0], 0);
        close(pfd[0]);
        runcmd(p);
      }
      close(pfd[0]);
      close(pfd[1]);
      await(buf, sizeof(buf));
      await(buf, sizeof(buf));
    } else {
      if(rfork(RFPROC|RFFDG) == 0)
        runcmd(buf);
      await(buf, sizeof(buf));
    }
  }
  exits(0);
}
