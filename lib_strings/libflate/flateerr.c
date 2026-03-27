/*s: libflate/flateerr.c */
#include <u.h>
#include <libc.h>
#include <flate.h>

/*s: function [[flateerr]] */
char *
flateerr(int err)
{
    switch(err){
    case FlateOk:
        return "no error";
    case FlateNoMem:
        return "out of memory";
    case FlateInputFail:
        return "input error";
    case FlateOutputFail:
        return "output error";
    case FlateCorrupted:
        return "corrupted data";
    case FlateInternal:
        return "internal error";
    }
    return "unknown error";
}
/*e: function [[flateerr]] */
/*e: libflate/flateerr.c */
