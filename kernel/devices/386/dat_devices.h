
/*
 *  hardware info about a device
 */
struct Devport {
  ulong port; 
  int size;
};

struct DevConf
{
  ulong intnum;     /* interrupt number */
  char  *type;      /* card type, malloced */
  int nports;     /* Number of ports */
  Devport *ports;     /* The ports themselves */
};
