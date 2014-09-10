// in libc, malloc.c

//Pool *mainmem = &sbrkmem;
//Pool *imagmem = &sbrkmem;
Pool*   imagmem = nil;
// for kernel stuff:
// globals: screenputs, vgacur, vgadev, 
// types: Pcidev
// functions: addphysseg(), mouseresize()

// and of course panic(), outb(), devxxx(), up, ...
