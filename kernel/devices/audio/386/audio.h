enum
{
  Bufsize = 1024, /* 5.8 ms each, must be power of two */
  Nbuf    = 128,  /* .74 seconds total */
  Dma   = 6,
  IrqAUDIO  = 7,
  SBswab  = 0,
};

#define seteisadma(a, b)  dmainit(a, Bufsize);
#define UNCACHED(type, v) (type*)((ulong)(v))

#define Int0vec 0
#define setvec(v, f, a)   arch_intrenable(v, f, a, BUSUNKNOWN, "audio")
