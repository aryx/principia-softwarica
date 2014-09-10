/*s: generators/lex/liblex/allprint.c */
#include	"../ldefs.h"
//#include	<stdio.h>

/*s: function printable */
//extern	FILE*	yyout;

int
printable(int c)
{
    return 040 < c && c < 0177;
}
/*e: function printable */

/*s: function allprint */
void
allprint(int c)
{
  exits("TODO allprint");
//	switch(c) {
//	case '\n':
//		fprintf(yyout,"\\n");
//		break;
//	case '\t':
//		fprintf(yyout,"\\t");
//		break;
//	case '\b':
//		fprintf(yyout,"\\b");
//		break;
//	case ' ':
//		fprintf(yyout,"\\\bb");
//		break;
//	default:
//		if(!printable(c))
//			fprintf(yyout,"\\%-3o",c);
//		else 
//			c = putc(c,yyout);
//			USED(c);
//		break;
//	}
//	return;
}
/*e: function allprint */
/*e: generators/lex/liblex/allprint.c */
