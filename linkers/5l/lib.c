/*s: linkers/5l/lib.c */
#include	"l.h"

/*s: global [[library]] */
// array<option<filename>>
char*	library[50];
/*e: global [[library]] */
/*s: global [[libraryobj]] */
char*	libraryobj[50];
/*e: global [[libraryobj]] */
/*s: global [[libraryp]] */
// index of first free entry in library array
int	libraryp;
/*e: global [[libraryp]] */

/*s: global [[libdir]] */
// growing_array<dirname>
char**	libdir;
/*e: global [[libdir]] */
/*s: global [[nlibdir]] */
// index of next free entry in libdir
int	nlibdir	= 0;
/*e: global [[nlibdir]] */
/*s: global [[maxlibdir]] */
// index of last free entry in libdir
static	int	maxlibdir = 0;
/*e: global [[maxlibdir]] */

/*s: function [[addlibpath]] */
void
addlibpath(char *arg)
{
    char **p;

    // growing array libdir
    if(nlibdir >= maxlibdir) {
        if(maxlibdir == 0)
            maxlibdir = 8;
        else
            maxlibdir *= 2;
        p = malloc(maxlibdir*sizeof(*p));
        /*s: [[addlibpath()]] sanity check p */
        if(p == nil) {
            diag("out of memory");
            errorexit();
        }
        /*e: [[addlibpath()]] sanity check p */
        memmove(p, libdir, nlibdir*sizeof(*p));
        free(libdir);
        libdir = p;
    }

    libdir[nlibdir++] = strdup(arg);
}
/*e: function [[addlibpath]] */

/*s: function [[findlib]] */
char*
findlib(char *file)
{
    int i;
    char name[LIBNAMELEN];

    for(i = 0; i < nlibdir; i++) {
        snprint(name, sizeof(name), "%s/%s", libdir[i], file);
        if(fileexists(name))
            return libdir[i];
    }
    return nil;
}
/*e: function [[findlib]] */

/*s: function [[loadlib]] */
void
loadlib(void)
{
    int i;
    long h;
    Sym *s;
loop:
    /*s: [[loadlib()]] reset xrefresolv */
    xrefresolv = false;
    /*e: [[loadlib()]] reset xrefresolv */
    for(i=0; i<libraryp; i++) {
        DBG("%5.2f autolib: %s (from %s)\n", cputime(), library[i], libraryobj[i]);
        objfile(library[i]);
    }
    /*s: [[loadlib()]] if xrefresolv */
    if(xrefresolv)
        for(h=0; h<nelem(hash); h++)
             for(s = hash[h]; s != S; s = s->link)
                 if(s->type == SXREF) {
                     DBG("symbol %s still not resolved, looping\n", s->name);//pad
                     goto loop;
                 }
    /*e: [[loadlib()]] if xrefresolv */
}
/*e: function [[loadlib]] */

/*s: function [[addlib]] */
/// ldobj(case AHISTORY and local_line == -1 special mark) -> <>
void
addlib(char *obj)
{
    char fn1[LIBNAMELEN], fn2[LIBNAMELEN], comp[LIBNAMELEN];
    char *p, *name;
    int i;
    bool search;

    if(histfrogp <= 0)
        return;

    name = fn1;
    search = false;
    if(histfrog[0]->name[1] == '/') {
        sprint(name, "");
        i = 1;
    } else if(histfrog[0]->name[1] == '.') {
        sprint(name, ".");
        i = 0;
    } else {
        sprint(name, "");
        i = 0;
        search = true;
    }

    for(; i<histfrogp; i++) {
        snprint(comp, sizeof comp, histfrog[i]->name+1);

        // s/$0/<thechar>/
        for(;;) {
            p = strstr(comp, "$O");
            if(p == nil)
                break;
            memmove(p+1, p+2, strlen(p+2)+1);
            p[0] = thechar;
        }
        // s/$M/<thestring>/
        for(;;) {
            p = strstr(comp, "$M");
            if(p == nil)
                break;
            if(strlen(comp)+strlen(thestring)-2+1 >= sizeof comp) {
                diag("library component too long");
                return;
            }
            memmove(p+strlen(thestring), p+2, strlen(p+2)+1);
            memmove(p, thestring, strlen(thestring));
        }

        if(strlen(fn1) + strlen(comp) + 3 >= sizeof(fn1)) {
            diag("library component too long");
            return;
        }
        if(i > 0 || !search)
            strcat(fn1, "/");
        strcat(fn1, comp);
    }

    cleanname(name);

    if(search){
        p = findlib(name);
        if(p != nil){
            snprint(fn2, sizeof(fn2), "%s/%s", p, name);
            name = fn2;
        }
    }


    for(i=0; i<libraryp; i++)
        if(strcmp(name, library[i]) == 0)
            return;
    if(libraryp == nelem(library)){
        diag("too many autolibs; skipping %s", name);
        return;
    }

    p = malloc(strlen(name) + 1);
    strcpy(p, name);
    library[libraryp] = p;
    p = malloc(strlen(obj) + 1);
    strcpy(p, obj);
    libraryobj[libraryp] = p;
    libraryp++;
}
/*e: function [[addlib]] */
/*e: linkers/5l/lib.c */
