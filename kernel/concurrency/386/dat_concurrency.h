
struct Lock
{
  ulong key; // 0 when unset, 0xDEADDEAD when acquired, could be a bool
  bool_ushort isilock; // false when from lock(), true when from ilock()

  ulong sr; // saved priority level when using ilock() to restore in iunlock()
  ulong pc; // for debugging

  // option<ref<Proc>>, None when key = 0
  Proc  *p; // the process locking should be the same unlocking
  // option<ref<Mach>>
  Mach  *m; // not that used, only in iprintcanlock apparently

  // debugging
//#ifdef LOCKCYCLES
  long  lockcycles;
//#endif
};
