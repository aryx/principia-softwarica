/*s: portdat_misc.h */

struct Cmdbuf
{
  char  *buf;
  char  **f;
  int nf;
};

struct Cmdtab
{
  int index;  /* used by client to switch on result */
  char  *cmd; /* command name */
  int narg; /* expected #args; 0 ==> variadic */
};
/*e: portdat_misc.h */
