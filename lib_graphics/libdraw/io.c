/*s: lib_graphics/libdraw/io.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function readimage */
Image*
readimage(Display *d, fdt fd, bool dolock)
{
    char hdr[5*12+1]; // 5 numbers as 11 chars plus space
    ulong chan;
    Rectangle r;
    Image *i;

    int miny, maxy;
    int dy;
    uint l, n;
    int m, j, chunk;
    byte *tmp;
    /*s: [[readimage()]] other locals */
    bool new;
    /*x: [[readimage()]] other locals */
    int ldepth;
    /*e: [[readimage()]] other locals */

    // Reading the header

    if(readn(fd, hdr, 11) != 11)
        return nil;
    /*s: [[readimage()]] if first 11 characters are compressed string */
    if(memcmp(hdr, "compressed\n", 11) == 0)
        return creadimage(d, fd, dolock);
    /*e: [[readimage()]] if first 11 characters are compressed string */
    // else
    if(readn(fd, hdr+11, 5*12-11) != 5*12-11)
        return nil;
    if(d)
        chunk = d->bufsize - 32;	/* a little room for header */
    /*s: [[readimage()]] set chunk if no display */
    else
        chunk = 8192;
    /*e: [[readimage()]] set chunk if no display */

    /*s: [[readimage()]] check if new or old format, set new */
    /*
     * distinguish new channel descriptor from old ldepth.
     * channel descriptors have letters as well as numbers,
     * while ldepths are a single digit formatted as %-11d.
     */
    new = false;
    for(m=0; m<10; m++){
        if(hdr[m] != ' '){
            new = true;
            break;
        }
    }
    /*e: [[readimage()]] check if new or old format, set new */
    /*s: [[readimage()]] sanity check hdr */
    if(hdr[11] != ' '){
        werrstr("readimage: bad format");
        return nil;
    }
    /*e: [[readimage()]] sanity check hdr */
    /*s: [[readimage()]] if old format */
    if(!new){
        ldepth = ((int)hdr[10])-'0';
        if(ldepth<0 || ldepth>3){
            werrstr("readimage: bad ldepth %d", ldepth);
            return nil;
        }
        chan = drawld2chan[ldepth];
    }
    /*e: [[readimage()]] if old format */
    else{
        hdr[11] = '\0';
        chan = strtochan(hdr);
        /*s: [[readimage()]] sanity check chan */
        if(chan == 0){
            werrstr("readimage: bad channel string %s", hdr);
            return nil;
        }
        /*e: [[readimage()]] sanity check chan */
    }

    r.min.x = atoi(hdr+1*12);
    r.min.y = atoi(hdr+2*12);
    r.max.x = atoi(hdr+3*12);
    r.max.y = atoi(hdr+4*12);
    /*s: [[readimage()]] sanity check r */
    if(r.min.x > r.max.x || r.min.y > r.max.y){
        werrstr("readimage: bad rectangle");
        return nil;
    }
    /*e: [[readimage()]] sanity check r */

    miny = r.min.y;
    maxy = r.max.y;

    // Allocating the image

    l = bytesperline(r, chantodepth(chan));
    if(d){
        /*s: [[readimage()]] lock display */
        if(dolock)
            lockdisplay(d);
        /*e: [[readimage()]] lock display */
        i = allocimage(d, r, chan, false, -1);
        /*s: [[readimage()]] unlock display */
        if(dolock)
            unlockdisplay(d);
        /*e: [[readimage()]] unlock display */
        /*s: [[readimage()]] sanity check i */
        if(i == nil)
            return nil;
        /*e: [[readimage()]] sanity check i */
    }
    /*s: [[readimage()]] alloc image if no display */
    else{
        i = mallocz(sizeof(Image), 1);
        if(i == nil)
            return nil;
    }
    /*e: [[readimage()]] alloc image if no display */

    // Read from disk and load the image

    tmp = malloc(chunk);
    /*s: [[readimage()]] sanity check tmp */
    if(tmp == nil)
        goto Err;
    /*e: [[readimage()]] sanity check tmp */

    while(maxy > miny){
        dy = maxy - miny;
        if(dy * l > chunk)
            dy = chunk/l;
        /*s: [[readimage()]] sanity check dy */
        if(dy <= 0){
            werrstr("readimage: image too wide for buffer");
            goto Err;
        }
        /*e: [[readimage()]] sanity check dy */
        n = dy * l;

        m = readn(fd, tmp, n);

        /*s: [[readimage()]] sanity check m */
        if(m != n){
            werrstr("readimage: read count %d not %d: %r", m, n);
        Err:
            if(dolock)
                lockdisplay(d);
        Err1:
            freeimage(i);
            if(dolock)
                unlockdisplay(d);
            free(tmp);
            return nil;
        }
        /*e: [[readimage()]] sanity check m */
        /*s: [[readimage()]] if old format, flip all bits */
        if(!new)	/* an old image: must flip all the bits */
            for(j=0; j<chunk; j++)
                tmp[j] ^= 0xFF;
        /*e: [[readimage()]] if old format, flip all bits */
        if(d){
            /*s: [[readimage()]] lock display */
            if(dolock)
                lockdisplay(d);
            /*e: [[readimage()]] lock display */

            if(loadimage(i, Rect(r.min.x, miny, r.max.x, miny+dy), tmp, 
                         chunk) <= 0)
                goto Err1;
            /*s: [[readimage()]] unlock display */
            if(dolock)
                unlockdisplay(d);
            /*e: [[readimage()]] unlock display */
        }
        miny += dy; // progress
    }
    free(tmp);
    return i;
}
/*e: function readimage */

// writeimage() is in compressed.c. It does not support the 
// regular (uncompressed) format.
/*e: lib_graphics/libdraw/io.c */
