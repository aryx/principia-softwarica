/*s: windows/libpanel/message.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>
#include "pldefs.h"
typedef struct Message Message;
/*s: struct Message */
struct Message{
    char *text;
    Point minsize;
};
/*e: struct Message */
/*s: function pl_textmsg */
void pl_textmsg(Image *b, Rectangle r, Font *f, char *s){
    char *start, *end;	/* of line */
    Point where;
    int lwid, c, wid;
    where=r.min;
    wid=r.max.x-r.min.x;
    do{
        start=s;
        lwid=0;
        end=s;
        do{
            for(;*s!=' ' && *s!='\0';s=pl_nextrune(s)) lwid+=pl_runewidth(f, s);
            if(lwid>wid) break;
            end=s;
            for(;*s==' ';s=pl_nextrune(s)) lwid+=pl_runewidth(f, s);
        }while(*s!='\0');
        if(end==start)	/* can't even fit one word on line! */
            end=s;
        c=*end;
        *end='\0';
        string(b, where, display->black, ZP, f, start);
        *end=c;
        where.y+=font->height;
        s=end;
        while(*s==' ') s=pl_nextrune(s);
    }while(*s!='\0');
}
/*e: function pl_textmsg */
/*s: function pl_foldsize */
Point pl_foldsize(Font *f, char *s, int wid){
    char *start, *end;	/* of line */
    Point size;
    int lwid, ewid;
    size=Pt(0,0);
    do{
        start=s;
        lwid=0;
        end=s;
        ewid=lwid;
        do{
            for(;*s!=' ' && *s!='\0';s=pl_nextrune(s)) lwid+=pl_runewidth(f, s);
            if(lwid>wid) break;
            end=s;
            ewid=lwid;
            for(;*s==' ';s=pl_nextrune(s)) lwid+=pl_runewidth(f, s);
        }while(*s!='\0');
        if(end==start){	/* can't even fit one word on line! */
            ewid=lwid;
            end=s;
        }
        if(ewid>size.x) size.x=ewid;
        size.y+=font->height;
        s=end;
        while(*s==' ') s=pl_nextrune(s);
    }while(*s!='\0');
    return size;
}
/*e: function pl_foldsize */
/*s: function pl_drawmessage */
void pl_drawmessage(Panel *p){
    pl_textmsg(p->b, pl_box(p->b, p->r, PASSIVE), font, ((Message *)p->data)->text);
}
/*e: function pl_drawmessage */
/*s: function pl_hitmessage */
int pl_hitmessage(Panel *g, Mouse *m){
    USED(g, m);
    return 0;
}
/*e: function pl_hitmessage */
/*s: function pl_typemessage */
void pl_typemessage(Panel *g, Rune c){
    USED(g, c);
}
/*e: function pl_typemessage */
/*s: function pl_getsizemessage */
Point pl_getsizemessage(Panel *p, Point children){
    Message *mp;
    USED(children);
    mp=p->data;
    return pl_boxsize(pl_foldsize(font, mp->text, mp->minsize.x), PASSIVE);
}
/*e: function pl_getsizemessage */
/*s: function pl_childspacemessage */
void pl_childspacemessage(Panel *p, Point *ul, Point *size){
    USED(p, ul, size);
}
/*e: function pl_childspacemessage */
/*s: function plinitmessage */
void plinitmessage(Panel *v, int flags, int wid, char *msg){
    Message *mp;
    mp=v->data;
    v->flags=flags|LEAF;
    v->draw=pl_drawmessage;
    v->hit=pl_hitmessage;
    v->type=pl_typemessage;
    v->getsize=pl_getsizemessage;
    v->childspace=pl_childspacemessage;
    mp->text=msg;
    mp->minsize=Pt(wid, font->height);
    v->kind="message";
}
/*e: function plinitmessage */
/*s: function plmessage */
Panel *plmessage(Panel *parent, int flags, int wid, char *msg){
    Panel *v;
    v=pl_newpanel(parent, sizeof(Message));
    plinitmessage(v, flags, wid, msg);
    return v;
}
/*e: function plmessage */
/*e: windows/libpanel/message.c */
