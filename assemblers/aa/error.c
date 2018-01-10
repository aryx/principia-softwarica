/*s: assemblers/aa/error.c */
#include "aa.h"

/*s: function [[errorexit]] */
/// main | assemble | yyerror | ... -> <>
void
errorexit(void)
{

    if(outfile)
        remove(outfile);
    exits("error");
}
/*e: function [[errorexit]] */

/*s: function [[yyerror]] */
/// assemble | yylex | yyparse | ... -> <>
void
yyerror(char *a, ...)
{
    char buf[200];
    va_list arg;

    /*s: [[yyerror()]] when called from yyparse */
    /*
     * hack to intercept message from yaccpar
     */
    if(strcmp(a, "syntax error") == 0) {
        yyerror("syntax error, last name: %s", symb);
        return;
    }
    /*e: [[yyerror()]] when called from yyparse */

    prfile(lineno);

    va_start(arg, a);
    vseprint(buf, buf+sizeof(buf), a, arg);
    va_end(arg);

    print("%s\n", buf);

    nerrors++;
    if(nerrors > 10) {
        print("too many errors\n");
        errorexit();
    }
}
/*e: function [[yyerror]] */

/*e: assemblers/aa/error.c */
