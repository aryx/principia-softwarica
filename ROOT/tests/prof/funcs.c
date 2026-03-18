#include <u.h>
#include <libc.h>

void bar(void) { /* some work */ }

void foo(void) { bar(); /* more work */ }

void main(void) {
    int i;
    for(i = 0; i < 100; i++)
        foo();
    bar();
    exits(nil);
}
