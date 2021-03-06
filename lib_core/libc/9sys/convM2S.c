/*s: libc/9sys/convM2S.c */
#include    <u.h>
#include    <libc.h>
#include    <fcall.h>

/*s: function [[gstring]] */
static
uchar*
gstring(uchar *p, uchar *ep, char **s)
{
    uint n;

    if(p+BIT16SZ > ep)
        return nil;
    n = GBIT16(p);
    p += BIT16SZ - 1;
    if(p+n+1 > ep)
        return nil;
    /* move it down, on top of count, to make room for '\0' */
    memmove(p, p + 1, n);
    p[n] = '\0';
    *s = (char*)p;
    p += n+1;
    return p;
}
/*e: function [[gstring]] */

/*s: function [[gqid]] */
static
uchar*
gqid(uchar *p, uchar *ep, Qid *q)
{
    if(p+QIDSZ > ep)
        return nil;
    q->type = GBIT8(p);
    p += BIT8SZ;
    q->vers = GBIT32(p);
    p += BIT32SZ;
    q->path = GBIT64(p);
    p += BIT64SZ;
    return p;
}
/*e: function [[gqid]] */

/*s: function [[convM2S]] */
/*
 * no syntactic checks.
 * three causes for error:
 *  1. message size field is incorrect
 *  2. input buffer too short for its own data (counts too long, etc.)
 *  3. too many names or qids
 * gqid() and gstring() return nil if they would reach beyond buffer.
 * main switch statement checks range and also can fall through
 * to test at end of routine.
 */
error0
convM2S(uchar *ap, uint nap, Fcall *f)
{
    uchar *p, *ep;
    uint i, size;

    p = ap;
    ep = p + nap;

    if(p+BIT32SZ+BIT8SZ+BIT16SZ > ep)
        return ERROR_0;
    // redo work done in read9pmsg
    size = GBIT32(p);
    p += BIT32SZ;

    if(size < BIT32SZ+BIT8SZ+BIT16SZ)
        return ERROR_0;

    f->type = GBIT8(p);
    p += BIT8SZ;
    f->tag = GBIT16(p);
    p += BIT16SZ;

    switch(f->type)
    {
    case Tversion:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->msize = GBIT32(p);
        p += BIT32SZ;
        p = gstring(p, ep, &f->version);
        break;

    case Tflush:
        if(p+BIT16SZ > ep)
            return ERROR_0;
        f->oldtag = GBIT16(p);
        p += BIT16SZ;
        break;

    case Tauth:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->afid = GBIT32(p);
        p += BIT32SZ;
        p = gstring(p, ep, &f->uname);
        if(p == nil)
            break;
        p = gstring(p, ep, &f->aname);
        if(p == nil)
            break;
        break;

    case Tattach:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->afid = GBIT32(p);
        p += BIT32SZ;
        p = gstring(p, ep, &f->uname);
        if(p == nil)
            break;
        p = gstring(p, ep, &f->aname);
        if(p == nil)
            break;
        break;

    case Twalk:
        if(p+BIT32SZ+BIT32SZ+BIT16SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        f->newfid = GBIT32(p);
        p += BIT32SZ;
        f->nwname = GBIT16(p);
        p += BIT16SZ;
        if(f->nwname > MAXWELEM)
            return ERROR_0;
        for(i=0; i<f->nwname; i++){
            p = gstring(p, ep, &f->wname[i]);
            if(p == nil)
                break;
        }
        break;

    case Topen:
        if(p+BIT32SZ+BIT8SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        f->mode = GBIT8(p);
        p += BIT8SZ;
        break;

    case Tcreate:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        p = gstring(p, ep, &f->name);
        if(p == nil)
            break;
        if(p+BIT32SZ+BIT8SZ > ep)
            return ERROR_0;
        f->perm = GBIT32(p);
        p += BIT32SZ;
        f->mode = GBIT8(p);
        p += BIT8SZ;
        break;

    case Tread:
        if(p+BIT32SZ+BIT64SZ+BIT32SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        f->offset = GBIT64(p);
        p += BIT64SZ;
        f->count = GBIT32(p);
        p += BIT32SZ;
        break;

    case Twrite:
        if(p+BIT32SZ+BIT64SZ+BIT32SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        f->offset = GBIT64(p);
        p += BIT64SZ;
        f->count = GBIT32(p);
        p += BIT32SZ;
        if(p+f->count > ep)
            return ERROR_0;
        f->data = (char*)p;
        p += f->count;
        break;

    case Tclunk:
    case Tremove:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        break;

    case Tstat:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        break;

    case Twstat:
        if(p+BIT32SZ+BIT16SZ > ep)
            return ERROR_0;
        f->fid = GBIT32(p);
        p += BIT32SZ;
        f->nstat = GBIT16(p);
        p += BIT16SZ;
        if(p+f->nstat > ep)
            return ERROR_0;
        f->stat = p;
        p += f->nstat;
        break;

/*
 */
    case Rversion:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->msize = GBIT32(p);
        p += BIT32SZ;
        p = gstring(p, ep, &f->version);
        break;

    case Rerror:
        p = gstring(p, ep, &f->ename);
        break;

    case Rflush:
        break;

    case Rauth:
        p = gqid(p, ep, &f->aqid);
        if(p == nil)
            break;
        break;

    case Rattach:
        p = gqid(p, ep, &f->qid);
        if(p == nil)
            break;
        break;

    case Rwalk:
        if(p+BIT16SZ > ep)
            return ERROR_0;
        f->nwqid = GBIT16(p);
        p += BIT16SZ;
        if(f->nwqid > MAXWELEM)
            return ERROR_0;
        for(i=0; i<f->nwqid; i++){
            p = gqid(p, ep, &f->wqid[i]);
            if(p == nil)
                break;
        }
        break;

    case Ropen:
    case Rcreate:
        p = gqid(p, ep, &f->qid);
        if(p == nil)
            break;
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->iounit = GBIT32(p);
        p += BIT32SZ;
        break;

    case Rread:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->count = GBIT32(p);
        p += BIT32SZ;
        if(p+f->count > ep)
            return ERROR_0;
        f->data = (char*)p;
        p += f->count;
        break;

    case Rwrite:
        if(p+BIT32SZ > ep)
            return ERROR_0;
        f->count = GBIT32(p);
        p += BIT32SZ;
        break;

    case Rclunk:
    case Rremove:
        break;

    case Rstat:
        if(p+BIT16SZ > ep)
            return ERROR_0;
        f->nstat = GBIT16(p);
        p += BIT16SZ;
        if(p+f->nstat > ep)
            return ERROR_0;
        f->stat = p;
        p += f->nstat;
        break;

    case Rwstat:
        break;
    default:
        return ERROR_0;
    }

    if(p==nil || p>ep)
        return ERROR_0;
    if(ap+size == p)
        return size;
    return ERROR_0;
}
/*e: function [[convM2S]] */
/*e: libc/9sys/convM2S.c */
