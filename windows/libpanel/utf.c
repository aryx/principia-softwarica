/*s: windows/libpanel/utf.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
/*s: function pl_idchar */
/*
 * This is the same definition that 8½ uses
 */
int pl_idchar(int c){
    if(c<=' '
    || 0x7F<=c && c<=0xA0
    || utfrune("!\"#$%&'()*+,-./:;<=>?@`[\\]^{|}~", c))
        return 0;
    return 1;
}
/*e: function pl_idchar */
/*s: function pl_rune1st */
int pl_rune1st(int c){
    return (c&0xc0)!=0x80;
}
/*e: function pl_rune1st */
/*s: function pl_nextrune */
char *pl_nextrune(char *s){
    do s++; while(!pl_rune1st(*s));
    return s;
}
/*e: function pl_nextrune */
/*s: function pl_runewidth */
int pl_runewidth(Font *f, char *s){
    char r[4], *t;
    t=r;
    do *t++=*s++; while(!pl_rune1st(*s));
    *t='\0';
    return stringwidth(f, r);
}
/*e: function pl_runewidth */
/*e: windows/libpanel/utf.c */
