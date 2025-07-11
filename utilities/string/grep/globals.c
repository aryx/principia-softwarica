/*s: grep/globals.c */
#include "grep.h"

union U u;

char    *filename;
char    *pattern;
Biobuf  bout;
char    flags[256];
Re**    follow;
ushort  gen;
char*   input;
long    lineno;
int literal;
int matched;
long    maxfollow;
long    nfollow;
int peekc;
Biobuf* rein;
State*  state0;
Re2 topre;
/*e: grep/globals.c */
