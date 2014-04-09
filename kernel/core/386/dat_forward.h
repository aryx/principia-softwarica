
typedef struct BIOS32si BIOS32si;
typedef struct BIOS32ci BIOS32ci;
typedef union FPsave  FPsave;
typedef struct FPssestate FPssestate;
typedef struct FPstate  FPstate;
typedef struct ISAConf  ISAConf;
typedef struct Label  Label;
typedef struct Lock Lock;
typedef struct MMU  MMU;
typedef struct Mach Mach;
typedef struct ArchNotsave  ArchNotsave;
typedef struct PCArch PCArch;
typedef struct Pcidev Pcidev;
typedef struct PCMmap PCMmap;
typedef struct PCMslot  PCMslot;
typedef struct Page Page;
typedef struct ArchMMU ArchMMU;
typedef struct Proc Proc;
typedef struct Segdesc  Segdesc;
typedef struct SFPssestate SFPssestate;
typedef vlong   Tval;
typedef struct Ureg Ureg;
typedef struct Vctl Vctl;

// was not there, but seems more consistent
typedef struct Tss Tss;
typedef struct Devport Devport;
// used to be in portdat.h
typedef struct Perf Perf;

#pragma incomplete BIOS32si
#pragma incomplete Pcidev
#pragma incomplete Ureg
