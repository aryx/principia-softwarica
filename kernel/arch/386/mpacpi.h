/*s: mpacpi.h */
/*
 * ACPI definitions
 *
 * A System Descriptor Table starts with a header of 4 bytes of signature
 * followed by 4 bytes of total table length then 28 bytes of ID information
 * (including the table checksum).
 */
typedef struct Dsdt Dsdt;
typedef struct Facp Facp;
typedef struct Hpet Hpet;
typedef struct Madt Madt;
typedef struct Mcfg Mcfg;
typedef struct Mcfgd Mcfgd;
typedef struct Rsd Rsd;

//@Scheck: type deps on params
struct Madt {       /* Multiple APIC DT */
  uchar sdthdr[36];   /* "MADT" + length[4] + [28] */
  uchar addr[4];    /* Local APIC Address */
  uchar flags[4];
  uchar structures[];
};
struct Rsd {        /* Root System Description * */
  uchar signature[8];   /* "RSD PTR " */
  uchar rchecksum;
  uchar oemid[6];
  uchar revision;
  uchar raddr[4];   /* RSDT */
  uchar length[4];
  uchar xaddr[8];   /* XSDT */
  uchar xchecksum;    /* XSDT */
  uchar pad[3];     /* reserved */
};
/*e: mpacpi.h */
