/*s: misc/echo.c */
#include <u.h>
#include <libc.h>

/*s: function [[main]](echo) */
void
main(int argc, char *argv[])
{
    bool nflag;
    int i, len;
    char *buf, *p;

    nflag = false;
    if(argc > 1 && strcmp(argv[1], "-n") == 0)
        nflag = true;

    len = 1;
    for(i = 1+nflag; i < argc; i++)
        len += strlen(argv[i])+1;

    buf = malloc(len);
    if(buf == 0)
        exits("no memory");

    p = buf;
    for(i = 1+nflag; i < argc; i++){
        strcpy(p, argv[i]);
        p += strlen(p);
        if(i < argc-1)
            *p++ = ' ';
    }
        
    if(!nflag)
        *p++ = '\n';

    if(write(STDOUT, buf, p-buf) < 0){
        fprint(STDERR, "echo: write error: %r\n");
        exits("write error");
    }

    exits((char *)nil);
}
/*e: function [[main]](echo) */
/*e: misc/echo.c */
