/*s: rc/getflags.c */
#include "getflags.h"

extern void	Exit(char*, char*);


static void reverse(char**, char**);
static int scanflag(int, char*);
static void errn(char*, int);
static void errs(char*);
static void errc(int);

/*s: global [[flagset]] */
char *flagset[] = {"<flag>"};
/*e: global [[flagset]] */
/*s: global [[flag]] */
// map<char, option<array<string>>>
char **flag[NFLAG];
/*e: global [[flag]] */
/*s: global [[cmdname]] */
char *cmdname;
/*e: global [[cmdname]] */
/*s: global [[flagarg]] */
static char *flagarg="";
/*e: global [[flagarg]] */
/*s: global [[reason]] */
static int reason;
/*e: global [[reason]] */

/*s: constant [[RESET]] */
#define	RESET	1
/*e: constant [[RESET]] */
/*s: constant [[FEWARGS]] */
#define	FEWARGS	2
/*e: constant [[FEWARGS]] */
/*s: constant [[FLAGSYN]] */
#define	FLAGSYN	3
/*e: constant [[FLAGSYN]] */
/*s: constant [[BADFLAG]] */
#define	BADFLAG	4
/*e: constant [[BADFLAG]] */

/*s: global [[badflag]] */
static int badflag;
/*e: global [[badflag]] */

/*s: function [[getflags]] */
int
getflags(int argc, char *argv[], char *flags, int stop)
{
    char *s;
    int i, j, c, count;
    flagarg = flags;
    if(cmdname==0)
        cmdname = argv[0];

    i = 1;
    while(i!=argc){
        if(argv[i][0] != '-' || argv[i][1] == '\0'){
            if(stop)		/* always true in rc */
                return argc;
            i++;
            continue;
        }
        s = argv[i]+1;
        while(*s){
            c=*s++;
            count = scanflag(c, flags);
            if(count==-1)
                return -1;
            if(flag[c]){ reason = RESET; badflag = c; return -1; }
            if(count==0){
                flag[c] = flagset;
                if(*s=='\0'){
                    for(j = i+1;j<=argc;j++)
                        argv[j-1] = argv[j];
                    --argc;
                }
            }
            else{
                if(*s=='\0'){
                    for(j = i+1;j<=argc;j++)
                        argv[j-1] = argv[j];
                    --argc;
                    s = argv[i];
                }
                if(argc-i<count){
                    reason = FEWARGS;
                    badflag = c;
                    return -1;
                }
                reverse(argv+i, argv+argc);
                reverse(argv+i, argv+argc-count);
                reverse(argv+argc-count+1, argv+argc);
                argc-=count;
                flag[c] = argv+argc+1;
                flag[c][0] = s;
                s="";
            }
        }
    }
    return argc;
}
/*e: function [[getflags]] */

/*s: function [[reverse]] */
static void
reverse(char **p, char **q)
{
    char *t;
    for(;p<q;p++,--q){ t=*p; *p=*q; *q = t; }
}
/*e: function [[reverse]] */

/*s: function [[scanflag]] */
static int
scanflag(int c, char *f)
{
    int fc, count;
    if(0<=c && c<NFLAG)
        while(*f){
            if(*f==' '){
                f++;
                continue;
            }
            fc=*f++;
            if(*f==':'){
                f++;
                if(*f<'0' || '9'<*f){ reason = FLAGSYN; return -1; }
                count = 0;
                while('0'<=*f && *f<='9') count = count*10+*f++-'0';
            }
            else
                count = 0;
            if(*f=='['){
                do{
                    f++;
                    if(*f=='\0'){ reason = FLAGSYN; return -1; }
                }while(*f!=']');
                f++;
            }
            if(c==fc)
                return count;
        }
    reason = BADFLAG;
    badflag = c;
    return -1;
}
/*e: function [[scanflag]] */

/*s: function [[usage]] */
void
usage(char *tail)
{
    char *s, *t, c;
    int count, nflag = 0;
    switch(reason){
    case RESET:
        errs("Flag -");
        errc(badflag);
        errs(": set twice\n");
        break;
    case FEWARGS:
        errs("Flag -");
        errc(badflag);
        errs(": too few arguments\n");
        break;
    case FLAGSYN:
        errs("Bad argument to getflags!\n");
        break;
    case BADFLAG:
        errs("Illegal flag -");
        errc(badflag);
        errc('\n');
        break;
    }
    errs("Usage: ");
    errs(cmdname);
    for(s = flagarg;*s;){
        c=*s;
        if(*s++==' ')
            continue;
        if(*s==':'){
            s++;
            count = 0;
            while('0'<=*s && *s<='9') count = count*10+*s++-'0';
        }
        else count = 0;
        if(count==0){
            if(nflag==0)
                errs(" [-");
            nflag++;
            errc(c);
        }
        if(*s=='['){
            s++;
            while(*s!=']' && *s!='\0') s++;
            if(*s==']')
                s++;
        }
    }
    if(nflag)
        errs("]");
    for(s = flagarg;*s;){
        c=*s;
        if(*s++==' ')
            continue;
        if(*s==':'){
            s++;
            count = 0;
            while('0'<=*s && *s<='9') count = count*10+*s++-'0';
        }
        else count = 0;
        if(count!=0){
            errs(" [-");
            errc(c);
            if(*s=='['){
                s++;
                t = s;
                while(*s!=']' && *s!='\0') s++;
                errs(" ");
                errn(t, s-t);
                if(*s==']')
                    s++;
            }
            else
                while(count--) errs(" arg");
            errs("]");
        }
        else if(*s=='['){
            s++;
            while(*s!=']' && *s!='\0') s++;
            if(*s==']')
                s++;
        }
    }
    if(tail){
        errs(" ");
        errs(tail);
    }
    errs("\n");
    Exit("bad flags", "getflags.c");
}
/*e: function [[usage]] */

/*s: function [[errn]] */
static void
errn(char *s, int count)
{
    while(count){ errc(*s++); --count; }
}
/*e: function [[errn]] */

/*s: function [[errs]] */
static void
errs(char *s)
{
    while(*s) errc(*s++);
}
/*e: function [[errs]] */
/*s: constant NBUF (rc/getflags.c) */
#define	NBUF	80
/*e: constant NBUF (rc/getflags.c) */
/*s: global [[buf]] */
static char buf[NBUF];
/*e: global [[buf]] */
/*s: global [[bufp]] */
static char *bufp = buf;
/*e: global [[bufp]] */

/*s: function [[errc]] */
static void
errc(int c)
{
    *bufp++=c;
    if(bufp==&buf[NBUF] || c=='\n'){
        write(2, buf, bufp-buf);
        bufp = buf;
    }
}
/*e: function [[errc]] */
/*e: rc/getflags.c */
