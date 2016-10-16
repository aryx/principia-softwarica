/*s: port/tokenize.c */
#include <u.h>
#include <libc.h>

/*s: global qsep */
static char qsep[] = " \t\r\n";
/*e: global qsep */

/*s: function qtoken */
static char*
qtoken(char *s, char *sep)
{
    int quoting;
    char *t;

    quoting = 0;
    t = s;  /* s is output string, t is input string */
    while(*t!='\0' && (quoting || utfrune(sep, *t)==nil)){
        if(*t != '\''){
            *s++ = *t++;
            continue;
        }
        /* *t is a quote */
        if(!quoting){
            quoting = 1;
            t++;
            continue;
        }
        /* quoting and we're on a quote */
        if(t[1] != '\''){
            /* end of quoted section; absorb closing quote */
            t++;
            quoting = 0;
            continue;
        }
        /* doubled quote; fold one quote into two */
        t++;
        *s++ = *t++;
    }
    if(*s != '\0'){
        *s = '\0';
        if(t == s)
            t++;
    }
    return t;
}
/*e: function qtoken */

/*s: function etoken */
static char*
etoken(char *t, char *sep)
{
    int quoting;

    /* move to end of next token */
    quoting = 0;
    while(*t!='\0' && (quoting || utfrune(sep, *t)==nil)){
        if(*t != '\''){
            t++;
            continue;
        }
        /* *t is a quote */
        if(!quoting){
            quoting = 1;
            t++;
            continue;
        }
        /* quoting and we're on a quote */
        if(t[1] != '\''){
            /* end of quoted section; absorb closing quote */
            t++;
            quoting = 0;
            continue;
        }
        /* doubled quote; fold one quote into two */
        t += 2;
    }
    return t;
}
/*e: function etoken */

/*s: function gettokens */
int
gettokens(char *s, char **args, int maxargs, char *sep)
{
    int nargs;

    for(nargs=0; nargs<maxargs; nargs++){
        while(*s!='\0' && utfrune(sep, *s)!=nil)
            *s++ = '\0';
        if(*s == '\0')
            break;
        args[nargs] = s;
        s = etoken(s, sep);
    }

    return nargs;
}
/*e: function gettokens */

/*s: function tokenize */
int
tokenize(char *s, char **args, int maxargs)
{
    int nargs;

    for(nargs=0; nargs<maxargs; nargs++){
        while(*s!='\0' && utfrune(qsep, *s)!=nil)
            s++;
        if(*s == '\0')
            break;
        args[nargs] = s;
        s = qtoken(s, qsep);
    }

    return nargs;
}
/*e: function tokenize */
/*e: port/tokenize.c */
