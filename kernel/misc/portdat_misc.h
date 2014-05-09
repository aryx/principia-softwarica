/*s: portdat_misc.h */

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
