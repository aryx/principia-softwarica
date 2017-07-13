/*s: windows/rio/processes_winshell.c */
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

/*s: function winshell */
void
winshell(void *args)
{
    Window *w;
    Channel *pidc;
    void **arg;
    char *cmd, *dir;
    char **argv;
    errorneg1 err;

    arg = args;

    w    = arg[0];
    pidc = arg[1];
    cmd  = arg[2];
    argv = arg[3];
    dir  = arg[4];

    // copy namespace/file-descriptors/environment-variables (do not share)
    rfork(RFNAMEG|RFFDG|RFENVG);

    /*s: [[winshell()]] adjust namespace */
    err = filsysmount(filsys, w->id);
    /*s: [[winshell()]] sanity check err filsysmount */
    if(err < 0){
        fprint(STDERR, "mount failed: %r\n");
        sendul(pidc, 0);
        threadexits("mount failed");
    }
    /*e: [[winshell()]] sanity check err filsysmount */
    /*e: [[winshell()]] adjust namespace */
    /*s: [[winshell()]] reassign STDIN/STDOUT after namespace adjustment */
    // reassign stdin/stdout to virtualized /dev/cons from filsysmount
    close(STDIN);
    err = open("/dev/cons", OREAD);
    /*s: [[winshell()]] sanity check err open cons stdin */
    if(err < 0){
        fprint(STDERR, "can't open /dev/cons: %r\n");
        sendul(pidc, 0);
        threadexits("/dev/cons");
    }
    /*e: [[winshell()]] sanity check err open cons stdin */
    close(STDOUT);
    err = open("/dev/cons", OWRITE);
    /*s: [[winshell()]] sanity check err open cons stdout */
    if(err < 0){
        fprint(STDERR, "can't open /dev/cons: %r\n");
        sendul(pidc, 0);
        threadexits("open");	/* BUG? was terminate() */
    }
    /*e: [[winshell()]] sanity check err open cons stdout */
    /*e: [[winshell()]] reassign STDIN/STDOUT after namespace adjustment */

    if(wclose(w) == false){	/* remove extra ref hanging from creation */
        notify(nil);
        dup(STDOUT, STDERR); // STDERR = STDOUT
        if(dir)
            chdir(dir);

        // Exec!!
        procexec(pidc, cmd, argv);
        _exits("exec failed"); // should never be reached
    }
}
/*e: function winshell */
/*e: windows/rio/processes_winshell.c */
