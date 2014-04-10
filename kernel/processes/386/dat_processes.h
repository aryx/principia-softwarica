
/*
 * the FP regs must be stored here, not somewhere pointed to from here.
 * port code assumes this.
 */
union FPsave {
  FPstate;
  SFPssestate;
};



/*
 *  things saved in the Proc structure during a notify
 */
//@Scheck: unnamed substructure
struct ArchProcNotsave
{
  ulong svflags;
  ulong svcs;
  ulong svss;
};
