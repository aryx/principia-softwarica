/*s: generators/lex/liblex/main.c */
#include	<u.h>
#include	<libc.h>
//#include	<stdio.h>

int	yylex(void);

/*s: function main */
void
main(int argc, char *argv[])
{
    USED(argc, argv);
    yylex();
    exits(0);
}
/*e: function main */
/*e: generators/lex/liblex/main.c */
