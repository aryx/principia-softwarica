/*s: core/386/dat_forward.h */
typedef union Arch_FPsave  Arch_FPsave;
typedef struct Arch_Proc Arch_Proc;

typedef struct BIOS32si BIOS32si;
typedef struct BIOS32ci BIOS32ci;
typedef struct FPssestate FPssestate;
typedef struct SFPssestate SFPssestate;
typedef struct FPstate  FPstate;
typedef struct MMU  MMU;
typedef struct PCArch PCArch;
typedef struct Pcidev Pcidev;
typedef struct PCMmap PCMmap;
typedef struct PCMslot  PCMslot;
typedef struct Segdesc  Segdesc;

typedef struct Ureg Ureg;
typedef struct Vctl Vctl;

// was not there, but seems more consistent
typedef struct Tss Tss;
typedef struct IOMap IOMap;
typedef struct X86type X86type;
typedef struct I8253 I8253;

// needed in arch specific stuff
typedef struct Page Page;
typedef struct Proc Proc;
typedef struct Lock Lock;

// was in io.h
typedef struct Pcisiz Pcisiz;
typedef struct Pcidev Pcidev;
typedef struct PCMconftab PCMconftab;

// was in mp.h
typedef struct PCMP PCMP;
typedef struct _MP_ _MP_;
typedef struct PCMPprocessor PCMPprocessor;
typedef struct PCMPbus PCMPbus;
typedef struct PCMPioapic PCMPioapic;
typedef struct PCMPintr PCMPintr;
typedef struct Aintr Aintr;
typedef struct Bus Bus;
typedef struct Apic Apic;

// was in mpacpi.h
typedef struct Dsdt Dsdt;
typedef struct Facp Facp;
typedef struct Hpet Hpet;
typedef struct Madt Madt;
typedef struct Mcfg Mcfg;
typedef struct Mcfgd Mcfgd;
typedef struct Rsd Rsd;


/*s: dat_forward.h pragma(x86) */
#pragma incomplete BIOS32si
#pragma incomplete Pcidev
#pragma incomplete Ureg
/*e: dat_forward.h pragma(x86) */
/*e: core/386/dat_forward.h */
