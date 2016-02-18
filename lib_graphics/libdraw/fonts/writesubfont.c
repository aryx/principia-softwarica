/*s: lib_graphics/libdraw/writesubfont.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function packinfo */
static
void
packinfo(Fontchar *fc, uchar *p, int n)
{
    int j;

    for(j=0;  j<=n;  j++){
        p[0] = fc->x;
        p[1] = fc->x>>8;
        p[2] = fc->top;
        p[3] = fc->bottom;
        p[4] = fc->left;
        p[5] = fc->width;
        fc++;
        p += 6;
    }
}
/*e: function packinfo */

/*s: function writesubfont */
errorneg1
writesubfont(fdt fd, Subfont *f)
{
    char hdr[3*12+1];
    byte *data;
    int nb;

    sprint(hdr, "%11d %11d %11d ", f->n, f->height, f->ascent);
    if(write(fd, hdr, 3*12) != 3*12){
    Err:
        werrstr("writesubfont: bad write: %r");
        return -1;
    }
    nb = 6*(f->n+1);
    data = malloc(nb);
    /*s: [[writesubfont()]] sanity check data */
    if(data == nil)
        return ERROR_NEG1;
    /*e: [[writesubfont()]] sanity check data */
    packinfo(f->info, data, f->n);
    if(write(fd, data, nb) != nb)
        goto Err;
    free(data);
    return OK_0;
}
/*e: function writesubfont */
/*e: lib_graphics/libdraw/writesubfont.c */
