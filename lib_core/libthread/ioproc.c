/*s: lib_core/libthread/ioproc.c */
#include <u.h>
#include <libc.h>
#include <thread.h>
#include "threadimpl.h"

/*s: enum _anon_ (lib_core/libthread/ioproc.c) */
enum
{
    STACK = 8192,
};
/*e: enum _anon_ (lib_core/libthread/ioproc.c) */

/*s: function iointerrupt */
void
iointerrupt(Ioproc *io)
{
    if(!io->inuse)
        return;
    threadint(io->tid);
}
/*e: function iointerrupt */

/*s: function xioproc */
static void
xioproc(void *a)
{
    Ioproc *io, *x;
    io = a;
    /*
     * first recvp acquires the ioproc.
     * second tells us that the data is ready.
     */
    for(;;){
        while(recv(io->c, &x) == -1)
            ;
        if(x == 0)	/* our cue to leave */
            break;
        assert(x == io);

        /* caller is now committed -- even if interrupted he'll return */
        while(recv(io->creply, &x) == -1)
            ;
        if(x == 0)	/* caller backed out */
            continue;
        assert(x == io);

        io->ret = io->op(&io->arg);
        if(io->ret < 0)
            rerrstr(io->err, sizeof io->err);
        while(send(io->creply, &io) == -1)
            ;
        while(recv(io->creply, &x) == -1)
            ;
    }
}
/*e: function xioproc */

/*s: function ioproc */
Ioproc*
ioproc(void)
{
    Ioproc *io;

    io = mallocz(sizeof(Ioproc), 1);
    if(io == nil)
        sysfatal("ioproc malloc: %r");
    io->c = chancreate(sizeof(void*), 0);
    io->creply = chancreate(sizeof(void*), 0);
    io->tid = proccreate(xioproc, io, STACK);
    return io;
}
/*e: function ioproc */

/*s: function closeioproc */
void
closeioproc(Ioproc *io)
{
    if(io == nil)
        return;
    iointerrupt(io);
    while(send(io->c, 0) == -1)
        ;
    chanfree(io->c);
    chanfree(io->creply);
    free(io);
}
/*e: function closeioproc */
/*e: lib_core/libthread/ioproc.c */
