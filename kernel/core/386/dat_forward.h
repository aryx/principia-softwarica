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

// was in io.h
typedef struct Pcisiz Pcisiz;
typedef struct Pcidev Pcidev;
typedef struct PCMslot    PCMslot;
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

// was in sd.h
typedef struct SDev SDev;
typedef struct SDifc SDifc;
typedef struct SDio SDio;
typedef struct SDpart SDpart;
typedef struct SDperm SDperm;
typedef struct SDreq SDreq;
typedef struct SDunit SDunit;

/*s: dat_forward.h pragma */
#pragma incomplete BIOS32si
#pragma incomplete Pcidev
#pragma incomplete Ureg
/*e: dat_forward.h pragma */
/*e: dat_forward.h */
