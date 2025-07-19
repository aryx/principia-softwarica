/*s: grep/globals.c */
#include "grep.h"

/*s: globals grep */
State*  state0;
// Top regexp?
Re2 topre;
/*x: globals grep */
union U u;
/*x: globals grep */
char    flags[256];
/*x: globals grep */
Biobuf  bout;
/*x: globals grep */
// option<ref_own<Biobuf>> for -f
Biobuf* rein;
/*x: globals grep */
// option<string>, for -f
char    *filename;
long    lineno;
/*x: globals grep */
char*   input;
char    *pattern;
ushort  gen;
/*x: globals grep */
// option<int> | EOF (-1)? (None = 0)
int peekc;
/*x: globals grep */
bool literal;
/*x: globals grep */
Re**    follow;
int matched;
long    maxfollow;
long    nfollow;
/*e: globals grep */
/*e: grep/globals.c */
