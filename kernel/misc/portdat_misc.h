/*s: portdat_misc.h */

enum misc_constants_portable {
    /* READSTR was 1000, which is way too small for usb's ctl file */
    READSTR = 4000,   /* temporary buffer size for device reads */

    KB = 1024,
    MB = (1024*1024),

    NUMSIZE = 12,   /* size of formatted number */

    /*s: constant PRINTSIZE */
    PRINTSIZE = 256,
    /*e: constant PRINTSIZE */
};


/*s: struct Cmdbuf */
struct Cmdbuf
{
  char  *buf;
  char  **f;
  int nf;
};
/*e: struct Cmdbuf */

/*s: struct Cmdtab */
struct Cmdtab
{
  int index;  /* used by client to switch on result */
  char  *cmd; /* command name */
  int narg; /* expected #args; 0 ==> variadic */
};
/*e: struct Cmdtab */
/*e: portdat_misc.h */
