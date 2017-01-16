/*s: aux.c */
#include <u.h>
#include <libc.h>
#include "../boot/boot.h"

// Note that most of this code is now superseded by $objtype/boot.rc

void
warning(char *s)
{
    char buf[ERRMAX];

    buf[0] = '\0';
    errstr(buf, sizeof buf);
    fprint(2, "boot: %s: %s\n", s, buf);
}

void
fatal(char *s)
{
    char *msg;
    char buf[ERRMAX];

    buf[0] = '\0';
    errstr(buf, sizeof buf);
    msg = smprint("%s: %s", s, buf);
    fprint(2, "boot: %s\n", msg);
    exits(msg);         /* this will trigger a panic */
}

/*s: function runv */
void
runv(char **argv)
{
  int i, pid;
  
  switch(pid = fork()){
  case -1:
    fatal("fork");
  case 0:
    exec(argv[0], argv);
    fatal(smprint("can't exec %s: %r", argv[0]));
  default:
    while ((i = waitpid()) != pid && i != -1)
      ;
    if(i == -1)
      fatal(smprint("wait failed running %s", argv[0]));
  }
}
/*e: function runv */

/*s: function run */
void
run(char *file, ...)
{
  runv(&file);
}
/*e: function run */


void bind_safe(char* old, char* new, int flag) {
  if(bind(old, new, flag) < 0)
    fatal("bind");
  return;
}

//less: could get path in fatal message
int open_safe(char* path, int flag) {
  int fd;
  if((fd = open(path, flag)) < 0) {
    fatal("open");
  }
  return fd;
}

void print_safe(int fd, char* str) {
  if(write(fd, str, strlen(str)) < 0) {
    fatal("print");
  };
  return;
}

void close_safe(int fd) {
  if(close(fd) < 0) {
    fatal("close");
  }
}
/*e: aux.c */
