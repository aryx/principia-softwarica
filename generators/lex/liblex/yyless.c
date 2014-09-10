/*s: generators/lex/liblex/yyless.c */
#include	<u.h>
#include	<libc.h>
//#include	<stdio.h>

extern	char	yytext[];
extern	int	yyleng;
extern	int	yyprevious;

void	yyunput(int c);

/*s: function yyless */
void
yyless(int x)
{
    char *lastch, *ptr;

    lastch = yytext+yyleng;
    if(x>=0 && x <= yyleng)
        ptr = x + yytext;
    else
        ptr = (char*)x;
    while(lastch > ptr)
        yyunput(*--lastch);
    *lastch = 0;
    if (ptr >yytext)
        yyprevious = lastch[-1];
    yyleng = ptr-yytext;
}
/*e: function yyless */
/*e: generators/lex/liblex/yyless.c */
