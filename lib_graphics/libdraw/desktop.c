/*s: lib_graphics/libdraw/desktop.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <window.h>
#include <draw_private.h>

/*s: global [[screenid]] */
static int	screenid;
/*e: global [[screenid]] */


/*s: function [[allocscreen]] */
Screen*
allocscreen(Image *image, Image *fill, bool public)
{
    Display *d;
    Screen *s;
    int id, try;
    byte *a;

    d = image->display;
    /*s: [[allocscreen()]] sanity check images have same display */
    if(d != fill->display){
        werrstr("allocscreen: image and fill on different displays");
        return nil;
    }
    /*e: [[allocscreen()]] sanity check images have same display */
    s = malloc(sizeof(Screen));
    /*s: [[allocscreen()]] sanity check s */
    if(s == nil)
        return nil;
    /*e: [[allocscreen()]] sanity check s */
    SET(id);
    for(try=0; try<25; try++){
        /* loop until find a free id */
        /*s: [[allocscreen()]] marshall allocate screen message */
        // allocate screen: 'A' id[4] imageid[4] fillid[4] public[1]
        a = bufimage(d, 1+4+4+4+1);
        /*s: [[allocscreen()]] sanity check a */
        if(a == nil){
            free(s);
            return nil;
        }
        /*e: [[allocscreen()]] sanity check a */
        id = ++screenid;
        a[0] = 'A';
        BPLONG(a+1, id);
        BPLONG(a+5, image->id);
        BPLONG(a+9, fill->id);
        a[13] = public;
        if(flushimage(d, false) != -1)
            break;
        /*e: [[allocscreen()]] marshall allocate screen message */
    }
    s->display = d;
    s->id = id;
    s->image = image;
    s->fill = fill;
    /*s: [[allocscreen()]] sanity check screen image */
    assert(s->image && s->image->chan != 0);
    /*e: [[allocscreen()]] sanity check screen image */
    return s;
}
/*e: function [[allocscreen]] */

/*s: function [[publicscreen]] */
Screen*
publicscreen(Display *d, int id, ulong chan)
{
    Screen *s;
    byte *a;

    s = malloc(sizeof(Screen));
    /*s: [[publicscreen()]] sanity check s */
    if(s == nil)
        return nil;
    /*e: [[publicscreen()]] sanity check s */

    /*s: [[publicscreen()]] marshall use public screen message */
    // use public screen: 'S' id[4] chan[4]
    a = bufimage(d, 1+4+4);
    /*s: [[publicscreen()]] sanity check a */
    if(a == nil){
    Error:
        free(s);
        return nil;
    }
    /*e: [[publicscreen()]] sanity check a */
    a[0] = 'S';
    BPLONG(a+1, id);
    BPLONG(a+5, chan);
    if(flushimage(d, false) < 0)
        goto Error;
    /*e: [[publicscreen()]] marshall use public screen message */

    s->display = d;
    s->id = id;

    s->image = nil;
    s->fill = nil;

    return s;
}
/*e: function [[publicscreen]] */

/*s: function [[freescreen]] */
errorneg1
freescreen(Screen *s)
{
    Display *d;
    byte *a;

    /*s: [[freescreen()]] sanity check s */
    if(s == nil)
        return 0;
    /*e: [[freescreen()]] sanity check s */
    d = s->display;
    /*s: [[freescreen()]] marshall free screen message */
    // free screen: 'F' id[4]
    a = bufimage(d, 1+4);
    /*s: [[freescreen()]] sanity check a */
    if(a == nil)
        return ERROR_NEG1;
    /*e: [[freescreen()]] sanity check a */
    a[0] = 'F';
    BPLONG(a+1, s->id);
    /*
     * flush(true) because screen is likely holding last reference to
     * window, and want it to disappear visually.
     */
    if(flushimage(d, true) < 0)
        return ERROR_NEG1;
    /*e: [[freescreen()]] marshall free screen message */
    free(s);
    return OK_1;
}
/*e: function [[freescreen]] */

/*e: lib_graphics/libdraw/desktop.c */
