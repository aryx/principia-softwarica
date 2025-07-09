/*s: files/mv.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

errorneg1 copy1(fdt fdf, fdt fdt, char *from, char *to);
void      hardremove(char *);
errorneg1 mv(char *from, char *todir, char *toelem);
errorneg1 mv1(char *from, Dir *dirb, char *todir, char *toelem);
bool      samefile(char *, char *);
void      split(char *, char **, char **);

/*s: function [[main]](mv.c) */
void
main(int argc, char *argv[])
{
    int i;
    errorn failed = OK_0;
    Dir *dirto, *dirfrom;
    char *todir, *toelem;

    if(argc<3){
        fprint(STDERR, "usage: mv fromfile tofile\n");
        fprint(STDERR, "   mv fromfile ... todir\n");
        exits("bad usage");
    }

    /* prepass to canonicalise names before splitting, etc. */
    for(i=1; i < argc; i++)
        cleanname(argv[i]);

    if((dirto = dirstat(argv[argc-1])) != nil && (dirto->mode&DMDIR)){
        dirfrom = nil;
        if(argc == 3
        && (dirfrom = dirstat(argv[1])) != nil
        && (dirfrom->mode & DMDIR)) 
            split(argv[argc-1], &todir, &toelem); /* mv dir1 dir2 */
        else{               /* mv file... dir */
            todir = argv[argc-1];
            toelem = nil;       /* toelem will be fromelem */
        }
        free(dirfrom);
    }else
        split(argv[argc-1], &todir, &toelem);   /* mv file1 file2 */
    free(dirto);
    if(argc>3 && toelem != nil){
        fprint(STDERR, "mv: %s not a directory\n", argv[argc-1]);
        exits("bad usage");
    }

    for(i=1; i < argc-1; i++)
        if(mv(argv[i], todir, toelem) == ERROR_NEG1)
            failed++;
    if(failed)
        exits("failure");
    exits(nil);
}
/*e: function [[main]](mv.c) */
/*s: function [[mv]] */
errorneg1
mv(char *from, char *todir, char *toelem)
{
    errorneg1 stat;
    Dir *dirb;

    dirb = dirstat(from);
    if(dirb == nil){
        fprint(STDERR, "mv: can't stat %s: %r\n", from);
        return ERROR_NEG1;
    }
    stat = mv1(from, dirb, todir, toelem);
    free(dirb);
    return stat;
}
/*e: function [[mv]] */
/*s: function [[mv1]] */
errorneg1
mv1(char *from, Dir *dirb, char *todir, char *toelem)
{
    // fd from, fd to
    fdt fdf, fdt;
    int i, j;
    errorneg1 stat;
    char toname[4096], fromname[4096];
    char *fromdir, *fromelem;
    Dir *dirt, null;

    strncpy(fromname, from, sizeof fromname);
    split(from, &fromdir, &fromelem);
    if(toelem == nil)
        toelem = fromelem;
    i = strlen(toelem);
    if(i==0){
        fprint(STDERR, "mv: null last name element moving %s\n", fromname);
        return ERROR_NEG1;
    }
    j = strlen(todir);
    if(i + j + 2 > sizeof toname){
        fprint(STDERR, "mv: path too big (max %d): %s/%s\n",
            sizeof toname, todir, toelem);
        return ERROR_NEG1;
    }
    memmove(toname, todir, j);
    toname[j] = '/';
    memmove(toname+j+1, toelem, i);
    toname[i+j+1] = '\0';

    if(samefile(fromdir, todir)){
        if(samefile(fromname, toname)){
            fprint(STDERR, "mv: %s and %s are the same\n",
                fromname, toname);
            return ERROR_NEG1;
        }

        /* remove target if present */
        dirt = dirstat(toname);
        if(dirt != nil) {
            hardremove(toname);
            free(dirt);
        }

        /* try wstat */
        nulldir(&null);
        null.name = toelem;
        if(dirwstat(fromname, &null) >= 0)
            return OK_0;
        if(dirb->mode & DMDIR){
            fprint(STDERR, "mv: can't rename directory %s: %r\n",
                fromname);
            return ERROR_NEG1;
        }
    }
    /*
     * Renaming won't work --- must copy
     */
    if(dirb->mode & DMDIR){
        fprint(STDERR, "mv: %s is a directory, not copied to %s\n",
            fromname, toname);
        return ERROR_NEG1;
    }
    fdf = open(fromname, OREAD);
    if(fdf < 0){
        fprint(STDERR, "mv: can't open %s: %r\n", fromname);
        return ERROR_NEG1;
    }

    dirt = dirstat(toname);
    if(dirt != nil && (dirt->mode & DMAPPEND))
        hardremove(toname);  /* because create() won't truncate file */
    free(dirt);

    fdt = create(toname, OWRITE, dirb->mode);
    if(fdt < 0){
        fprint(STDERR, "mv: can't create %s: %r\n", toname);
        close(fdf);
        return ERROR_NEG1;
    }
    stat = copy1(fdf, fdt, fromname, toname);
    close(fdf);

    if(stat >= 0){
        nulldir(&null);
        null.mtime = dirb->mtime;
        null.mode = dirb->mode;
        dirfwstat(fdt, &null);  /* ignore errors; e.g. user none always fails */
        if(remove(fromname) < 0){
            fprint(STDERR, "mv: can't remove %s: %r\n", fromname);
            stat = ERROR_NEG1;
        }
    }
    close(fdt);
    return stat;
}
/*e: function [[mv1]] */
/*s: function [[copy1]](mv.c) */
errorneg1
copy1(fdt fdf, fdt fdt, char *from, char *to)
{
    char buf[8192];
    long n, n1;

    while ((n = read(fdf, buf, sizeof buf)) > 0) {
        n1 = write(fdt, buf, n);
        if(n1 != n){
            fprint(STDERR, "mv: error writing %s: %r\n", to);
            return ERROR_NEG1;
        }
    }
    if(n < 0){
        fprint(STDERR, "mv: error reading %s: %r\n", from);
        return ERROR_NEG1;
    }
    return OK_0;
}
/*e: function [[copy1]](mv.c) */
/*s: function [[split]](mv.c) */
void
split(char *name, char **pdir, char **pelem)
{
    char *s;

    s = utfrrune(name, '/');
    if(s){
        *s = '\0';
        *pelem = s+1;
        *pdir = name;
    }else if(strcmp(name, "..") == 0){
        *pdir = "..";
        *pelem = ".";
    }else{
        *pdir = ".";
        *pelem = name;
    }
}
/*e: function [[split]](mv.c) */
/*s: function [[samefile]](mv.c) */
bool
samefile(char *a, char *b)
{
    Dir *da, *db;
    bool ret;

    if(strcmp(a, b) == 0)
        return true;
    da = dirstat(a);
    db = dirstat(b);
    ret = (da != nil && db != nil &&
        da->qid.type==db->qid.type &&
        da->qid.path==db->qid.path &&
        da->qid.vers==db->qid.vers &&
        da->dev==db->dev &&
        da->type==db->type);
    free(da);
    free(db);
    return ret;
}
/*e: function [[samefile]](mv.c) */
/*s: function [[hardremove]](mv.c) */
void
hardremove(char *a)
{
    if(remove(a) == ERROR_NEG1){
        fprint(STDERR, "mv: can't remove %s: %r\n", a);
        exits("mv");
    }
    //????
    while(remove(a) != ERROR_NEG1)
        ;
}
/*e: function [[hardremove]](mv.c) */
/*e: files/mv.c */
