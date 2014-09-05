/*s: windows/codegraph.h */

// in libc, malloc.c
//Pool *mainmem = &sbrkmem;
/*s: global imagmem */
//Pool *imagmem = &sbrkmem;
Pool*   imagmem = nil;
/*e: global imagmem */


// for kernel stuff:
// globals: screenputs, vgacur, vgadev, 
// types: Pcidev
// functions: addphysseg(), mouseresize()

// and of course panic(), outb(), devxxx(), up, ...

/*e: windows/codegraph.h */
