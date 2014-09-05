/*s: lib_graphics/libdraw/scroll.c */
#include <u.h>
#include <libc.h>
#include <draw.h>

/*s: function mousescrollsize */
int
mousescrollsize(int maxlines)
{
    static int lines, pcnt;
    char *mss;

    if(lines == 0 && pcnt == 0){
        mss = getenv("mousescrollsize");
        if(mss){
            if(strchr(mss, '%') != nil)
                pcnt = atof(mss);
            else
                lines = atoi(mss);
            free(mss);
        }
        if(lines == 0 && pcnt == 0)
            lines = 1;
        if(pcnt>=100)
            pcnt = 100;
    }

    if(lines)
        return lines;
    return pcnt * maxlines/100.0;	
}
/*e: function mousescrollsize */
/*e: lib_graphics/libdraw/scroll.c */
