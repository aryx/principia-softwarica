#include <stdlib.h>
#include <unistd.h>
#include <draw.h>
#include <event.h>

void  eresized(int x) {
    print("eresized %d\n", x);
}

void main(void) {
  
  int res;

  res = initdraw(nil, nil, "test_draw");
  if (res < 0) {
    print("error in initdraw\n");
    exit(0);
  }

  //sleep(5);

  einit(Ekeyboard);

  for(;;) {
    res = ekbd();
    print("key =%d\n", res);
  }

}
