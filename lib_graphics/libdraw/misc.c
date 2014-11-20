/*s: lib_graphics/libdraw/misc.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: global drawld2chan */
ulong drawld2chan[] = {
    GREY1,
    GREY2,
    GREY4,
    CMAP8,
};
/*e: global drawld2chan */

/*s: function setalpha */
ulong
setalpha(ulong color, uchar alpha)
{
    int red, green, blue;

    red = (color >> 3*8) & 0xFF;
    green = (color >> 2*8) & 0xFF;
    blue = (color >> 1*8) & 0xFF;
    /* ignore incoming alpha */
    red = (red * alpha)/255;
    green = (green * alpha)/255;
    blue = (blue * alpha)/255;
    return (red<<3*8) | (green<<2*8) | (blue<<1*8) | (alpha<<0*8);
}
/*e: function setalpha */
/*e: lib_graphics/libdraw/misc.c */
