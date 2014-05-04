/*s: iomap.c */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "io.h"

struct Iomapalloc iomap;

// to remove some backward dependencies, so that ioalloc()
// can be here instead of in devarch.c
void (*hook_ioalloc)() = nil;

void
ioinit(void)
{
    char *excluded;
    int i;

    for(i = 0; i < nelem(iomap.maps)-1; i++)
        iomap.maps[i].next = &iomap.maps[i+1];
    iomap.maps[i].next = nil;
    iomap.free = iomap.maps;

    /*
     * This is necessary to make the IBM X20 boot.
     * Have not tracked down the reason.
     * i82557 is at 0x1000, the dummy entry is needed for swappable devs.
     */
    ioalloc(0x0fff, 1, 0, "dummy");

    if ((excluded = getconf("ioexclude")) != nil) {
        char *s;

        s = excluded;
        while (s && *s != '\0' && *s != '\n') {
            char *ends;
            int io_s, io_e;

            io_s = (int)strtol(s, &ends, 0);
            if (ends == nil || ends == s || *ends != '-') {
                print("ioinit: cannot parse option string\n");
                break;
            }
            s = ++ends;

            io_e = (int)strtol(s, &ends, 0);
            if (ends && *ends == ',')
                *ends++ = '\0';
            s = ends;

            ioalloc(io_s, io_e - io_s + 1, 0, "pre-allocated");
        }
    }

}


/*
 *  alloc some io port space and remember who it was
 *  alloced to.  if port < 0, find a free region.
 */
int
ioalloc(int port, int size, int align, char *tag)
{
    IOMap *m, **l;
    int i;

    lock(&iomap);
    if(port < 0){
        /* find a free port above 0x400 and below 0x1000 */
        port = 0x400;
        for(l = &iomap.m; *l; l = &(*l)->next){
            m = *l;
            if (m->start < 0x400) continue;
            i = m->start - port;
            if(i > size)
                break;
            if(align > 0)
                port = ((port+align-1)/align)*align;
            else
                port = m->end;
        }
        if(*l == nil){
            unlock(&iomap);
            return -1;
        }
    } else {
        /* Only 64KB I/O space on the x86. */
        if((port+size) > 0x10000){
            unlock(&iomap);
            return -1;
        }
        /* see if the space clashes with previously allocated ports */
        for(l = &iomap.m; *l; l = &(*l)->next){
            m = *l;
            if(m->end <= port)
                continue;
            if(m->reserved && m->start == port && m->end == port + size) {
                m->reserved = 0;
                unlock(&iomap);
                return m->start;
            }
            if(m->start >= port+size)
                break;
            unlock(&iomap);
            return -1;
        }
    }
    m = iomap.free;
    if(m == nil){
        print("ioalloc: out of maps");
        unlock(&iomap);
        return port;
    }
    iomap.free = m->next;
    m->next = *l;
    m->start = port;
    m->end = port + size;
    strncpy(m->tag, tag, sizeof(m->tag));
    m->tag[sizeof(m->tag)-1] = 0;
    *l = m;

    //archdir[0].qid.vers++;
        if(hook_ioalloc) 
            hook_ioalloc();

    unlock(&iomap);
    return m->start;
}

void
iofree(int port)
{
    IOMap *m, **l;

    lock(&iomap);
    for(l = &iomap.m; *l; l = &(*l)->next){
        if((*l)->start == port){
            m = *l;
            *l = m->next;
            m->next = iomap.free;
            iomap.free = m;
            break;
        }
        if((*l)->start > port)
            break;
    }
    //archdir[0].qid.vers++;
        if(hook_ioalloc) 
            hook_ioalloc();

    unlock(&iomap);
}

///*
// * Reserve a range to be ioalloced later.
// * This is in particular useful for exchangable cards, such
// * as pcmcia and cardbus cards.
// */
//int
//ioreserve(int, int size, int align, char *tag)
//{
//  IOMap *m, **l;
//  int i, port;
//
//  lock(&iomap);
//  /* find a free port above 0x400 and below 0x1000 */
//  port = 0x400;
//  for(l = &iomap.m; *l; l = &(*l)->next){
//      m = *l;
//      if (m->start < 0x400) continue;
//      i = m->start - port;
//      if(i > size)
//          break;
//      if(align > 0)
//          port = ((port+align-1)/align)*align;
//      else
//          port = m->end;
//  }
//  if(*l == nil){
//      unlock(&iomap);
//      return -1;
//  }
//  m = iomap.free;
//  if(m == nil){
//      print("ioalloc: out of maps");
//      unlock(&iomap);
//      return port;
//  }
//  iomap.free = m->next;
//  m->next = *l;
//  m->start = port;
//  m->end = port + size;
//  m->reserved = 1;
//  strncpy(m->tag, tag, sizeof(m->tag));
//  m->tag[sizeof(m->tag)-1] = 0;
//  *l = m;
//
//  archdir[0].qid.vers++;
//
//  unlock(&iomap);
//  return m->start;
//}

/*e: iomap.c */
