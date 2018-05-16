#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>

static Panel *root, *list;

char *genlist(Panel *, int which){
	static char buf[7];
	if(which<0 || 26<=which) return 0;
	sprint(buf, "item %c", which+'a');
	return buf;
}

void hitgen(Panel *p, int buttons, int sel){
	USED(p, buttons, sel);
}

void eresized(bool new){
    Rectangle r;
	if(new && getwindow(display, Refnone) == -1) {
		fprint(STDERR, "getwindow: %r\n");
		exits("getwindow");
	}
	r=view->r;
	r=insetrect(r, 4);
	plpack(root, r);
	pldraw(root, view);
}

void done(Panel *p, int buttons){
	USED(p, buttons);
	exits(0);
}

Panel *msg;
void message(char *s, ...){
	char buf[1024];
    int n;
	va_list arg;

	va_start(arg, s);
	n = snprint(buf, sizeof(buf), s, arg);
	va_end(arg);
	buf[n]='\0';
	plinitlabel(msg, PACKN|FILLX, buf);
	pldraw(msg, view);
}

Scroll s;

void save(Panel *p, int buttons){
	USED(p, buttons);
	s=plgetscroll(list);
	message("save %d %d %d %d", s);
}

void revert(Panel *p, int buttons){
	USED(p, buttons);
	plsetscroll(list, s);
	message("revert %d %d %d %d", s);
}

void main(void){
    Mouse m;
	Panel *g;

    if(initdraw(0, 0, "scrltest") < 0)
      sysfatal("initdraw: %r");
	einit(Emouse);
	plinit(view->depth);

	root=plgroup(0, 0);
	g=plgroup(root, PACKN|EXPAND);
	list=pllist(g, PACKE|EXPAND, genlist, 8, hitgen);
	plscroll(list, 0, plscrollbar(g, PACKW));
	msg=pllabel(root, PACKN|FILLX, "");
	plbutton(root, PACKW, "save", save);
	plbutton(root, PACKW, "revert", revert);
	plbutton(root, PACKE, "done", done);

    eresized(false);

	for(;;) {
      m = emouse();
      plmouse(root, &m);
    }
}
