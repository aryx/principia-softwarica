/*s: windows/rio/thread_keyboard.c */
#include <u.h>
#include <libc.h>

// for dat.h
#include <draw.h>
#include <mouse.h>
#include <cursor.h>
#include <keyboard.h>
#include <frame.h>
#include <fcall.h>
#include <thread.h>

#include "dat.h"
#include "fns.h"

/*s: function [[keyboardthread]] */
void
keyboardthread(void*)
{
    Rune buf[2][20];
    // points to buf[0] or buf[1]
    Rune *rp;
    int n, i;

    threadsetname("keyboardthread");

    n = 0;
    for(;;){
        rp = buf[n];
        n = 1-n;

        // Listen
        recv(keyboardctl->c, rp);

        for(i=1; i<nelem(buf[0])-1; i++)
            if(nbrecv(keyboardctl->c, rp+i) <= 0)
                break;
        rp[i] = L'\0';

        if(input != nil)
            // Dispatch, to current window thread!
            sendp(input->ck, rp);
    }
}
/*e: function [[keyboardthread]] */
/*e: windows/rio/thread_keyboard.c */
