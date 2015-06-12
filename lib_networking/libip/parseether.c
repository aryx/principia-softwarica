/*s: lib_networking/libip/parseether.c */
#include <u.h>
#include <libc.h>

/*s: function parseether */
errorneg1
parseether(uchar* to, char *from)
{
    char nip[4];
    char *p;
    int i;

    p = from;
    for(i = 0; i < 6; i++){
        if(*p == '\0')
            return ERROR_NEG1;
        nip[0] = *p++;
        if(*p == '\0')
            return ERROR_NEG1;
        nip[1] = *p++;
        nip[2] = '\0';
        to[i] = strtoul(nip, 0, 16);
        if(*p == ':')
            p++;
    }
    return OK_0;
}
/*e: function parseether */
/*e: lib_networking/libip/parseether.c */
