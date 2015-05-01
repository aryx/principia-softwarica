#include <stdlib.h>
#include <unistd.h>
#include <draw.h>

void main(void) {
  
  int res;

  res = initdraw(nil, nil, "test_draw");
  if (res < 0) {
    print("error in initdraw\n");
    exit(0);
  }
  sleep(5);

}
