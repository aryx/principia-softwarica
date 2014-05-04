/*s: dat_forward.h */
typedef union ArchFPsave  ArchFPsave;
typedef struct ArchProcNotsave  ArchProcNotsave;
typedef struct ArchProcMMU ArchProcMMU;

typedef struct BIOS32si BIOS32si;
typedef struct BIOS32ci BIOS32ci;
typedef struct FPssestate FPssestate;
typedef struct SFPssestate SFPssestate;
typedef struct FPstate  FPstate;
typedef struct ISAConf  ISAConf;
typedef struct MMU  MMU;
typedef struct PCArch PCArch;
typedef struct Pcidev Pcidev;
typedef struct PCMmap PCMmap;
typedef struct PCMslot  PCMslot;

typedef struct Segdesc  Segdesc;
typedef vlong   Tval;
typedef struct Ureg Ureg;
typedef struct Vctl Vctl;

// was not there, but seems more consistent
typedef struct Tss Tss;
typedef struct Devport Devport;
typedef struct Vctl Vctl;
typedef struct IOMap IOMap;
typedef struct X86type X86type;
typedef struct I8253 I8253;

// needed in arch specific stuff
typedef struct Page Page;
typedef struct Proc Proc;
typedef struct Lock Lock;

#pragma incomplete BIOS32si
#pragma incomplete Pcidev
#pragma incomplete Ureg
/*e: dat_forward.h */
