
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

