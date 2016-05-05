/*s: lib_graphics/libdraw/stringwidth.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>
#include <font.h>

/*s: enum _anon_ (lib_graphics/libdraw/stringwidth.c) */
enum { Max = 64 };
/*e: enum _anon_ (lib_graphics/libdraw/stringwidth.c) */

/*s: function _stringnwidth */
int
_stringnwidth(Font *f, char *s, Rune *r, int len)
{
    Rune **rptr;
    char **sptr;
    int wid, twid;
    int n, max, l;
    ushort cbuf[Max];
    char *subfontname;
    /*s: [[_stringnwidth()]] other locals */
    char *name;
    Rune rune;
    /*x: [[_stringnwidth()]] other locals */
    Font *def;
    /*e: [[_stringnwidth()]] other locals */

    /*s: [[_string()]] non unicode string handling, set sptr and rptr */
    if(s == nil){
        s = "";
        sptr = nil;
    }else
        sptr = &s;

    if(r == nil){
        r = (Rune*) L"";
        rptr = nil;
    }
    /*e: [[_string()]] non unicode string handling, set sptr and rptr */
    else
        rptr = &r;

    twid = 0;
    while(len > 0 && (*s || *r)){
        /*s: [[_string()]] set max */
        max = Max;
        if(len < max)
            max = len;
        /*e: [[_string()]] set max */
        n = 0;
        while((l = cachechars(f, sptr, rptr, cbuf, max, &wid, &subfontname)) <= 0){
            /*s: [[_stringnwidth()]] if cachechars failed too much */
            if(++n > 10){
                if(*r)
                    rune = *r;
                else
                    chartorune(&rune, s);
                if(f->name != nil)
                    name = f->name;
                else
                    name = "unnamed font";
                fprint(2, "stringwidth: bad character set for rune 0x%.4ux in %s\n", rune, name);
                return twid;
            }
            /*e: [[_stringnwidth()]] if cachechars failed too much */
            /*s: [[_stringnwidth()]] if subfontname */
            if(subfontname){
                if(_getsubfont(f->display, subfontname) == 0){
                    def = f->display->defaultfont;
                    if(def && f!=def)
                        f = def;
                    else
                        break;
                }
            }
            /*e: [[_stringnwidth()]] if subfontname */
        }
        agefont(f);

        twid += wid;
        len -= l; // progress
    }
    return twid;
}
/*e: function _stringnwidth */

/*s: function stringnwidth */
int
stringnwidth(Font *f, char *s, int len)
{
    return _stringnwidth(f, s, nil, len);
}
/*e: function stringnwidth */

/*s: function stringwidth */
int
stringwidth(Font *f, char *s)
{
    return _stringnwidth(f, s, nil, 1<<24);
}
/*e: function stringwidth */

/*s: function stringsize */
Point
stringsize(Font *f, char *s)
{
    return Pt(_stringnwidth(f, s, nil, 1<<24), f->height);
}
/*e: function stringsize */

/*s: function runestringnwidth */
int
runestringnwidth(Font *f, Rune *r, int len)
{
    return _stringnwidth(f, nil, r, len);
}
/*e: function runestringnwidth */

/*s: function runestringwidth */
int
runestringwidth(Font *f, Rune *r)
{
    return _stringnwidth(f, nil, r, 1<<24);
}
/*e: function runestringwidth */

/*s: function runestringsize */
Point
runestringsize(Font *f, Rune *r)
{
    return Pt(_stringnwidth(f, nil, r, 1<<24), f->height);
}
/*e: function runestringsize */
/*e: lib_graphics/libdraw/stringwidth.c */
