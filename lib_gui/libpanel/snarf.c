/*s: lib_gui/libpanel/snarf.c */
/*s: [[libpanel]] includes */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>

#include <panel.h>
#include "pldefs.h"
/*e: [[libpanel]] includes */

/*s: function [[plputsnarf]] */
void plputsnarf(char *s){
    int fd;

    if(s==0 || *s=='\0')
        return;
    if((fd=open("/dev/snarf", OWRITE|OTRUNC))>=0){
        write(fd, s, strlen(s));
        close(fd);
    }
}
/*e: function [[plputsnarf]] */
/*s: function [[plgetsnarf]] */
char *plgetsnarf(void){
    int fd, n, r;
    char *s;

    if((fd=open("/dev/snarf", OREAD))<0)
        return nil;
    n=0;
    s=nil;
    for(;;){
        s=pl_erealloc(s, n+1024);
        if((r = read(fd, s+n, 1024)) <= 0)
            break;
        n += r;
    }
    close(fd);
    if(n <= 0){
        free(s);
        return nil;
    }
    s[n] = '\0';
    return s;
}
/*e: function [[plgetsnarf]] */
/*s: function [[plsnarf]] */
void plsnarf(Panel *p){
    char *s;

    if(p==0 || p->snarf==0)
        return;
    s=p->snarf(p);
    plputsnarf(s);
    free(s);
}
/*e: function [[plsnarf]] */
/*s: function [[plpaste]] */
void plpaste(Panel *p){
    char *s;

    if(p==0 || p->paste==0)
        return;
    if(s=plgetsnarf()){
        p->paste(p, s);
        free(s);
    }
}
/*e: function [[plpaste]] */
/*e: lib_gui/libpanel/snarf.c */
