#include <u.h>
#include <libc.h>

void main(){
  char* args[] = {"/"};
  exec("/bin/ls", args);
}
