/*s: networking/ip/imap4d/utils.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <auth.h>
#include "imap4d.h"

/*s: function [[strrev]] */
/*
 * reverse string [s:e) in place
 */
void
strrev(char *s, char *e)
{
    int c;

    while(--e > s){
        c = *s;
        *s++ = *e;
        *e = c;
    }
}
/*e: function [[strrev]] */

/*s: function [[isdotdot]] */
int
isdotdot(char *s)
{
    return s[0] == '.' && s[1] == '.' && (s[2] == '/' || s[2] == '\0');
}
/*e: function [[isdotdot]] */

/*s: function [[issuffix]] */
int
issuffix(char *suf, char *s)
{
    int n;

    n = strlen(s) - strlen(suf);
    if(n < 0)
        return 0;
    return strcmp(s + n, suf) == 0;
}
/*e: function [[issuffix]] */

/*s: function [[isprefix]] */
int
isprefix(char *pre, char *s)
{
    return strncmp(pre, s, strlen(pre)) == 0;
}
/*e: function [[isprefix]] */

/*s: function [[ciisprefix]] */
int
ciisprefix(char *pre, char *s)
{
    return cistrncmp(pre, s, strlen(pre)) == 0;
}
/*e: function [[ciisprefix]] */

/*s: function [[readFile]] */
char*
readFile(int fd)
{
    Dir *d;
    long length;
    char *s;

    d = dirfstat(fd);
    if(d == nil)
        return nil;
    length = d->length;
    free(d);
    s = binalloc(&parseBin, length + 1, 0);
    if(s == nil || read(fd, s, length) != length)
        return nil;
    s[length] = '\0';
    return s;
}
/*e: function [[readFile]] */

/*s: function [[imapTmp]] */
/*
 * create the imap tmp file.
 * it just happens that we don't need multiple temporary files.
 */
int
imapTmp(void)
{
    char buf[ERRMAX], name[MboxNameLen];
    int tries, fd;

    snprint(name, sizeof(name), "/mail/box/%s/mbox.tmp.imp", username);
    for(tries = 0; tries < LockSecs*2; tries++){
        fd = create(name, ORDWR|ORCLOSE|OCEXEC, DMEXCL|0600);
        if(fd >= 0)
            return fd;
        errstr(buf, sizeof buf);
        if(cistrstr(buf, "locked") == nil)
            break;
        sleep(500);
    }
    return -1;
}
/*e: function [[imapTmp]] */

/*s: function [[openLocked]]([[(networking/ip/imap4d/utils.c)]]) */
/*
 * open a file which might be locked.
 * if it is, spin until available
 */
int
openLocked(char *dir, char *file, int mode)
{
    char buf[ERRMAX];
    int tries, fd;

    for(tries = 0; tries < LockSecs*2; tries++){
        fd = cdOpen(dir, file, mode);
        if(fd >= 0)
            return fd;
        errstr(buf, sizeof buf);
        if(cistrstr(buf, "locked") == nil)
            break;
        sleep(500);
    }
    return -1;
}
/*e: function [[openLocked]]([[(networking/ip/imap4d/utils.c)]]) */

/*s: function [[fqid]] */
int
fqid(int fd, Qid *qid)
{
    Dir *d;

    d = dirfstat(fd);
    if(d == nil)
        return -1;
    *qid = d->qid;
    free(d);
    return 0;
}
/*e: function [[fqid]] */

/*s: function [[mapInt]] */
ulong
mapInt(NamedInt *map, char *name)
{
    int i;

    for(i = 0; map[i].name != nil; i++)
        if(cistrcmp(map[i].name, name) == 0)
            break;
    return map[i].v;
}
/*e: function [[mapInt]] */

/*s: function [[estrdup]]([[(networking/ip/imap4d/utils.c)]]) */
char*
estrdup(char *s)
{
    char *t;

    t = emalloc(strlen(s) + 1);
    strcpy(t, s);
    return t;
}
/*e: function [[estrdup]]([[(networking/ip/imap4d/utils.c)]]) */

/*s: function [[emalloc]]([[(networking/ip/imap4d/utils.c)]]) */
void*
emalloc(ulong n)
{
    void *p;

    p = malloc(n);
    if(p == nil)
        bye("server out of memory");
    setmalloctag(p, getcallerpc(&n));
    return p;
}
/*e: function [[emalloc]]([[(networking/ip/imap4d/utils.c)]]) */

/*s: function [[ezmalloc]] */
void*
ezmalloc(ulong n)
{
    void *p;

    p = malloc(n);
    if(p == nil)
        bye("server out of memory");
    setmalloctag(p, getcallerpc(&n));
    memset(p, 0, n);
    return p;
}
/*e: function [[ezmalloc]] */

/*s: function [[erealloc]]([[(networking/ip/imap4d/utils.c)]]) */
void*
erealloc(void *p, ulong n)
{
    p = realloc(p, n);
    if(p == nil)
        bye("server out of memory");
    setrealloctag(p, getcallerpc(&p));
    return p;
}
/*e: function [[erealloc]]([[(networking/ip/imap4d/utils.c)]]) */
/*e: networking/ip/imap4d/utils.c */
