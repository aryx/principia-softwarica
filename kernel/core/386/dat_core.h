
struct ArchConf {
  // 386/ specific?
  ulong base0;    /* base of bank 0 */
  ulong base1;    /* base of bank 1 */
};



struct ArchMach {
  // 386 specific part
  // TODO: have a MMMU like in bcm/
  ulong*  pdb;      /* page directory base for this processor (va) */
  Tss*  tss;      /* tss for this processor */
  Segdesc *gdt;     /* gdt for this processor */
  Proc* externup;   /* extern register Proc *up */
  Page* pdbpool;
  int pdbcnt;
  int inclockintr;


  // 386/ specific
  int loopconst;
  int cpuidax;
  int cpuiddx;
  char  cpuidid[16];
  char* cpuidtype;
  int havetsc;
  int havepge;
  uvlong tscticks;
  int pdballoc;
  int pdbfree;
  vlong mtrrcap;
  vlong mtrrdef;
  vlong mtrrfix[11];
  vlong mtrrvar[32];    /* 256 max. */

};

/*
 *  routines for things outside the PC model, like power management
 */
// this is actually only used in 386/ code.
struct PCArch
{
  char* id;
  int (*ident)(void);   /* this should be in the model */
  void  (*reset)(void);   /* this should be in the model */
  int (*serialpower)(int);  /* 1 == on, 0 == off */
  int (*modempower)(int); /* 1 == on, 0 == off */

  void  (*intrinit)(void);
  int (*intrenable)(Vctl*);
  int (*intrvecno)(int);
  int (*intrdisable)(int);
  void  (*introff)(void);
  void  (*intron)(void);

  void  (*clockenable)(void);
  uvlong  (*fastclock)(uvlong*);
  void  (*timerset)(uvlong);

  void  (*resetothers)(void); /* put other cpus into reset */
};
extern PCArch *arch;      /* PC architecture */
