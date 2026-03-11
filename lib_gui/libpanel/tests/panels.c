/* Claude Code
 *
 * Copyright (C) 2026 Yoann Padioleau
 *
 * All libpanel widgets in a single window, for a quick overview screenshot.
 */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>

Panel *root;
Panel *status;

static void statusmsg(char *fmt, ...){
	static char buf[256];
	va_list arg;
	va_start(arg, fmt);
	vsnprint(buf, sizeof buf, fmt, arg);
	va_end(arg);
	plinitlabel(status, PACKS|FILLX, buf);
	pldraw(status, view);
}

/* ---- callbacks ---- */

static void btn_hit(Panel *p, int buttons){
	USED(p);
	statusmsg("Button clicked (buttons=%d)", buttons);
}

static void check_hit(Panel *p, int buttons, int val){
	USED(p, buttons);
	statusmsg("Check: %s", val ? "on" : "off");
}

static void radio_hit(Panel *p, int buttons, int val){
	USED(p, buttons);
	statusmsg("Radio: %s", val ? "selected" : "deselected");
}

static void entry_hit(Panel *p, char *text){
	USED(p);
	statusmsg("Entry: %s", text);
}

static void slider_hit(Panel *p, int buttons, int val, int len){
	USED(p, buttons);
	statusmsg("Slider: %d/%d", val, len);
}

static void canvas_draw(Panel *p){
	Rectangle r;
	Point c;
	int rad;

	r = p->r;
	draw(p->b, r, display->white, nil, ZP);
	/* diagonal cross */
	line(p->b, r.min, r.max, 0, 0, 0, display->black, ZP);
	line(p->b, Pt(r.max.x, r.min.y), Pt(r.min.x, r.max.y),
		0, 0, 0, display->black, ZP);
	/* centered circle */
	c = divpt(addpt(r.min, r.max), 2);
	rad = (r.max.x - r.min.x) / 4;
	if(rad > (r.max.y - r.min.y) / 4)
		rad = (r.max.y - r.min.y) / 4;
	if(rad > 0)
		ellipse(p->b, c, rad, rad, 0, display->black, ZP);
}

static void canvas_hit(Panel *p, Mouse *m){
	USED(p);
	statusmsg("Canvas: (%d,%d)", m->xy.x, m->xy.y);
}

static char *listgen(Panel *p, int which){
	static char buf[32];
	USED(p);
	if(which < 0 || which >= 10)
		return nil;
	snprint(buf, sizeof buf, "item %c", which + 'a');
	return buf;
}

static void list_hit(Panel *p, int buttons, int which){
	USED(p, buttons);
	statusmsg("List item %d", which);
}

static void do_exit(Panel *p, int buttons){
	USED(p, buttons);
	exits(nil);
}

/* ---- main ---- */

void eresized(bool new){
	if(new && getwindow(display, Refnone) < 0){
		fprint(STDERR, "getwindow: %r\n");
		exits("getwindow");
	}
	plpack(root, view->r);
	pldraw(root, view);
}

void main(void){
	Event e;
	Panel *f, *g, *c, *l;

	if(initdraw(nil, nil, "panels") < 0)
		sysfatal("initdraw: %r");
	einit(Emouse|Ekeyboard);
	plinit(view->depth);

	root = plgroup(nil, NOFLAG);

	/* Title */
	pllabel(root, PACKN|FILLX, "libpanel Widget Gallery");

	/* ---- Row 1: Labels ---- */
	f = plframe(root, PACKN|FILLX);
	pllabel(f, PACKN|FILLX, "Labels:");
	pllabel(f, PACKN|FILLX, "  A simple text label");
	pllabel(f, PACKN|FILLX, "  Another label");

	/* ---- Row 2: Buttons ---- */
	f = plframe(root, PACKN|FILLX);
	pllabel(f, PACKN|FILLX, "Buttons:");
	g = plgroup(f, PACKN|FILLX);
	plbutton(g, PACKW, "Button 1", btn_hit);
	plbutton(g, PACKW, "Button 2", btn_hit);
	plbutton(g, PACKW, "Button 3", btn_hit);

	/* ---- Row 3: Checkboxes & Radio ---- */
	f = plframe(root, PACKN|FILLX);
	g = plgroup(f, PACKW);
	pllabel(g, PACKN|FILLX, "Checkboxes:");
	plcheckbutton(g, PACKN, "Option A", check_hit);
	plcheckbutton(g, PACKN, "Option B", check_hit);
	plcheckbutton(g, PACKN, "Option C", check_hit);
	g = plgroup(f, PACKW);
	pllabel(g, PACKN|FILLX, "Radio buttons:");
	plradiobutton(g, PACKN, "Choice 1", radio_hit);
	plradiobutton(g, PACKN, "Choice 2", radio_hit);
	plradiobutton(g, PACKN, "Choice 3", radio_hit);

	/* ---- Row 4: Entry ---- */
	f = plframe(root, PACKN|FILLX);
	pllabel(f, PACKN|FILLX, "Text entry:");
	plentry(f, PACKN|FILLX, 250, "type here", entry_hit);

	/* ---- Row 5: Sliders ---- */
	f = plframe(root, PACKN|FILLX);
	pllabel(f, PACKN|FILLX, "Sliders:");
	g = plgroup(f, PACKN|FILLX);
	pllabel(g, PACKW, "H:");
	plslider(g, PACKW|FILLX|EXPAND, Pt(150, 20), slider_hit);
	plslider(f, PACKN, Pt(20, 80), slider_hit);

	/* ---- Row 6: Canvas ---- */
	f = plframe(root, PACKN|FILLX);
	pllabel(f, PACKN|FILLX, "Canvas:");
	c = plcanvas(f, PACKN|FILLX|EXPAND, canvas_draw, canvas_hit);
	c->ipad = Pt(5, 5);
	c->fixedsize = Pt(200, 80);

	/* ---- Row 7: List with scrollbar ---- */
	f = plframe(root, PACKN|FILLX);
	pllabel(f, PACKN|FILLX, "List with scrollbar:");
	g = plgroup(f, PACKN|FILLX);
	l = pllist(g, PACKE|FILLX|EXPAND, listgen, 5, list_hit);
	plscroll(l, nil, plscrollbar(g, PACKW));

	/* ---- Bottom: exit + status ---- */
	plbutton(root, PACKS|PLACESE, "Exit", do_exit);
	status = pllabel(root, PACKS|FILLX, "Ready");

	eresized(false);

	for(;;){
		switch(event(&e)){
		case Emouse:
			plmouse(root, &e.mouse);
			break;
		case Ekeyboard:
			plkeyboard(e.kbdc);
			break;
		}
	}
}
