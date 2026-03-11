/* Claude Code
 *
 * Copyright (C) 2026 Yoann Padioleau
 *
 * Widget gallery for libpanel -- each menu item opens a window
 * showcasing a specific widget type, useful for book screenshots.
 *
 * Inspired by GTK's gtk3-widget-factory.
 */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <event.h>
#include <panel.h>

Panel *root;
Panel *demo;		/* frame where demos are shown */
Panel *status;		/* status label at bottom */

static void statusmsg(char *fmt, ...){
	static char buf[256];
	va_list arg;
	va_start(arg, fmt);
	vsnprint(buf, sizeof buf, fmt, arg);
	va_end(arg);
	plinitlabel(status, PACKN|FILLX, buf);
	pldraw(status, view);
}

/*
 * Rebuild the demo area with new content.
 * We free the old demo frame and create a new one,
 * then repack and redraw.
 */
static void rebuild(void){
	plpack(root, view->r);
	pldraw(root, view);
}

static void cleardemo(void){
	Panel *c;
	/* free all children of demo */
	if(demo->child){
		plfree(demo->child);
		demo->child = nil;
		demo->echild = nil;
	}
}

/*
 * ---- Label demo ----
 */
static void show_labels(int buttons, int idx){
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "A simple text label");
	pllabel(demo, PACKN|FILLX, "Labels are the simplest widget.");
	pllabel(demo, PACKN|FILLX, "They display text and ignore all events.");
	rebuild();
	statusmsg("Labels: passive text display");
}

/*
 * ---- Button demo ----
 */
static void btn_hit(Panel *p, int buttons){
	USED(p);
	statusmsg("Button clicked (buttons=%d)", buttons);
}

static void show_buttons(int buttons, int idx){
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Buttons:");
	plbutton(demo, PACKN, "Click me", btn_hit);
	plbutton(demo, PACKN, "Another button", btn_hit);
	plbutton(demo, PACKN, "A third one", btn_hit);
	rebuild();
	statusmsg("Buttons: click to activate");
}

/*
 * ---- Checkbox demo ----
 */
static void check_hit(Panel *p, int buttons, int val){
	USED(p, buttons);
	statusmsg("Checkbox toggled: %s", val ? "checked" : "unchecked");
}

static void show_checkboxes(int buttons, int idx){
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Checkboxes:");
	plcheckbutton(demo, PACKN, "Option A", check_hit);
	plcheckbutton(demo, PACKN, "Option B", check_hit);
	plcheckbutton(demo, PACKN, "Option C", check_hit);
	rebuild();
	statusmsg("Checkboxes: toggle on/off");
}

/*
 * ---- Radio button demo ----
 */
static void radio_hit(Panel *p, int buttons, int val){
	USED(p, buttons);
	statusmsg("Radio: %s", val ? "selected" : "deselected");
}

static void show_radios(int buttons, int idx){
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Radio buttons (mutually exclusive):");
	plradiobutton(demo, PACKN, "Choice 1", radio_hit);
	plradiobutton(demo, PACKN, "Choice 2", radio_hit);
	plradiobutton(demo, PACKN, "Choice 3", radio_hit);
	rebuild();
	statusmsg("Radio buttons: only one can be selected");
}

/*
 * ---- Entry demo ----
 */
static void entry_hit(Panel *p, char *text){
	USED(p);
	statusmsg("Entry submitted: %s", text);
}

static void show_entry(int buttons, int idx){
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Text entry (type and press Enter):");
	plentry(demo, PACKN|FILLX, 200, "edit me", entry_hit);
	pllabel(demo, PACKN|FILLX, "Another entry:");
	plentry(demo, PACKN|FILLX, 200, "", entry_hit);
	rebuild();
	statusmsg("Entry: single-line text input");
}

/*
 * ---- Slider demo ----
 */
static void hslider_hit(Panel *p, int buttons, int val, int len){
	USED(p, buttons);
	statusmsg("Horizontal slider: %d / %d", val, len);
}

static void vslider_hit(Panel *p, int buttons, int val, int len){
	USED(p, buttons);
	statusmsg("Vertical slider: %d / %d", val, len);
}

static void show_sliders(int buttons, int idx){
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Horizontal slider:");
	plslider(demo, PACKN|FILLX, Pt(200, 20), hslider_hit);
	pllabel(demo, PACKN|FILLX, "Vertical slider:");
	plslider(demo, PACKN, Pt(20, 150), vslider_hit);
	rebuild();
	statusmsg("Sliders: drag to set value");
}

/*
 * ---- Canvas demo ----
 */
static void canvas_draw(Panel *p){
	Rectangle r;
	Point c;
	int rad;

	r = p->r;
	draw(p->b, r, display->white, nil, ZP);

	/* draw a diagonal cross */
	line(p->b, r.min, r.max, 0, 0, 0, display->black, ZP);
	line(p->b, Pt(r.max.x, r.min.y), Pt(r.min.x, r.max.y),
		0, 0, 0, display->black, ZP);

	/* draw a circle in the center */
	c = divpt(addpt(r.min, r.max), 2);
	rad = (r.max.x - r.min.x) / 4;
	if(rad > (r.max.y - r.min.y) / 4)
		rad = (r.max.y - r.min.y) / 4;
	ellipse(p->b, c, rad, rad, 0, display->black, ZP);
}

static void canvas_hit(Panel *p, Mouse *m){
	USED(p);
	statusmsg("Canvas click at (%d, %d) buttons=%d", m->xy.x, m->xy.y, m->buttons);
}

static void show_canvas(int buttons, int idx){
	Panel *c;
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Canvas (custom drawing area):");
	c = plcanvas(demo, PACKN|FILLX|FILLY|EXPAND, canvas_draw, canvas_hit);
	c->ipad = Pt(5, 5);
	rebuild();
	statusmsg("Canvas: application draws here");
}

/*
 * ---- Frame demo ----
 */
static void show_frames(int buttons, int idx){
	Panel *f1, *f2;
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Frames (containers with borders):");
	f1 = plframe(demo, PACKN|FILLX);
	pllabel(f1, PACKN|FILLX, "Inside frame 1");
	pllabel(f1, PACKN|FILLX, "Second line in frame 1");
	f2 = plframe(demo, PACKN|FILLX);
	pllabel(f2, PACKN|FILLX, "Inside frame 2");
	plbutton(f2, PACKN, "Button in frame", btn_hit);
	rebuild();
	statusmsg("Frames: decorative container");
}

/*
 * ---- Group demo ----
 */
static void show_groups(int buttons, int idx){
	Panel *g1, *g2;
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Groups (invisible containers):");
	g1 = plgroup(demo, PACKN|FILLX);
	pllabel(g1, PACKN, "Group 1: packed north");
	plbutton(g1, PACKN, "G1 button", btn_hit);
	g2 = plgroup(demo, PACKN|FILLX);
	pllabel(g2, PACKN, "Group 2: packed north");
	plcheckbutton(g2, PACKN, "G2 checkbox", check_hit);
	rebuild();
	statusmsg("Groups: invisible layout container");
}

/*
 * ---- List demo ----
 */
static char *listgen(Panel *p, int which){
	static char buf[32];
	USED(p);
	if(which < 0 || which >= 20)
		return nil;
	snprint(buf, sizeof buf, "List item %d", which);
	return buf;
}

static void list_hit(Panel *p, int buttons, int which){
	USED(p, buttons);
	statusmsg("List: selected item %d", which);
}

static void show_list(int buttons, int idx){
	Panel *g, *l;
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Scrollable list:");
	g = plgroup(demo, PACKN|FILLX|FILLY|EXPAND);
	l = pllist(g, PACKE|FILLX|FILLY|EXPAND, listgen, 8, list_hit);
	plscroll(l, nil, plscrollbar(g, PACKW));
	rebuild();
	statusmsg("List: scrollable item list");
}

/*
 * ---- Scrollbar demo ----
 */
static void show_scrollbar(int buttons, int idx){
	Panel *g, *l;
	USED(buttons, idx);
	cleardemo();
	pllabel(demo, PACKN|FILLX, "Scrollbar with list:");
	g = plgroup(demo, PACKN|FILLX|FILLY|EXPAND);
	l = pllist(g, PACKE|FILLX|FILLY|EXPAND, listgen, 5, list_hit);
	plscroll(l, plscrollbar(g, PACKS), plscrollbar(g, PACKW));
	rebuild();
	statusmsg("Scrollbar: scroll bars linked to a list");
}

/*
 * ---- Exit ----
 */
static void do_exit(int buttons, int idx){
	USED(buttons, idx);
	exits(nil);
}

/*
 * Main menu
 */
static Icon *menuitems[] = {
	"Label",
	"Button",
	"Checkbox",
	"Radio button",
	"Entry",
	"Slider",
	"Canvas",
	"Frame",
	"Group",
	"List",
	"Scrollbar",
	"Exit",
	nil,
};

static void menuhit(int buttons, int idx){
	switch(idx){
	case 0:  show_labels(buttons, idx); break;
	case 1:  show_buttons(buttons, idx); break;
	case 2:  show_checkboxes(buttons, idx); break;
	case 3:  show_radios(buttons, idx); break;
	case 4:  show_entry(buttons, idx); break;
	case 5:  show_sliders(buttons, idx); break;
	case 6:  show_canvas(buttons, idx); break;
	case 7:  show_frames(buttons, idx); break;
	case 8:  show_groups(buttons, idx); break;
	case 9:  show_list(buttons, idx); break;
	case 10: show_scrollbar(buttons, idx); break;
	case 11: do_exit(buttons, idx); break;
	}
}

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
	Panel *menu;

	if(initdraw(nil, nil, "panels_test") < 0)
		sysfatal("initdraw: %r");
	einit(Emouse|Ekeyboard);
	plinit(view->depth);

	root = plgroup(nil, NOFLAG);

	/* title */
	pllabel(root, PACKN|FILLX, "libpanel Widget Gallery");

	/* menu bar at top */
	menu = plmenu(root, PACKN|FILLX, menuitems, PACKW, menuhit);
	USED(menu);

	/* demo area in the center */
	demo = plframe(root, PACKN|FILLX|FILLY|EXPAND);
	pllabel(demo, PACKN|FILLX, "Select a widget from the menu above.");

	/* status bar at bottom */
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
