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

void ereshaped(Rectangle r){
	view.r=r;
	r=inset(r, 4);
	plpack(root, r);
	bitblt(&view, view.r.min, &view, view.r, Zero);
	pldraw(root, &view);
}

void done(Panel *p, int buttons){
	USED(p, buttons);
	bitblt(&view, view.r.min, &view, view.r, Zero);
	exits(0);
}

Panel *msg;
void message(char *s, ...){
	char buf[1024], *out;
	va_list arg;
	va_start(arg, s);
	out = doprint(buf, buf+sizeof(buf), s, arg);
	va_end(arg);
	*out='\0';
	plinitlabel(msg, PACKN|FILLX, buf);
	pldraw(msg, &view);
}

Scroll s;

void save(Panel *p, int buttons){
	USED(p, buttons);
	s=plgetscroll(list);
	message("save %d %d %d %d", s);
}

void revert(Panel *p, int buttons){
	USED(p, buttons);
	plsetscroll(list, s, &view);
	message("revert %d %d %d %d", s);
}

void main(void){
	Panel *g;

	binit(0,0,0);
	einit(Emouse);
	plinit(view.ldepth);

	root=plgroup(0, 0);
	g=plgroup(root, PACKN|EXPAND);
	list=pllist(g, PACKE|EXPAND, genlist, 8, hitgen);
	plscroll(list, 0, plscrollbar(g, PACKW));
	msg=pllabel(root, PACKN|FILLX, "");
	plbutton(root, PACKW, "save", save);
	plbutton(root, PACKW, "revert", revert);
	plbutton(root, PACKE, "done", done);

	ereshaped(view.r);
	for(;;) 
      plmouse(root, emouse(), &view);
}
