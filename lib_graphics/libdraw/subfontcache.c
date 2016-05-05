/*s: lib_graphics/libdraw/subfontcache.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <font.h>

/*
 * Easy versions of the cache routines; may be substituted by fancier ones for other purposes
 */

/*s: global lastname */
static char	*lastname;
/*e: global lastname */
/*s: global lastsubfont */
Subfont	*lastsubfont;
/*e: global lastsubfont */

/*s: function lookupsubfont */
Subfont*
lookupsubfont(Display *d, char *name)
{
    if(d && strcmp(name, "*default*") == 0)
        return d->defaultsubfont; 
    if(lastname && strcmp(name, lastname)==0)
      if(d == lastsubfont->bits->display){
        lastsubfont->ref++;
        return lastsubfont;
    }
    return nil;
}
/*e: function lookupsubfont */

/*s: function installsubfont */
void
installsubfont(char *name, Subfont *subfont)
{
    free(lastname);
    lastname = strdup(name);
    lastsubfont = subfont;	/* notice we don't free the old one; that's your business */
}
/*e: function installsubfont */

/*s: function uninstallsubfont */
void
uninstallsubfont(Subfont *subfont)
{
    if(subfont == lastsubfont){
        lastname = nil;
        lastsubfont = nil;
    }
}
/*e: function uninstallsubfont */
/*e: lib_graphics/libdraw/subfontcache.c */
