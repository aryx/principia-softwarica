/*s: portkbd.h */
/*s: portkbd.h forward decl */
typedef struct Kbscan Kbscan;
/*e: portkbd.h forward decl */

/*s: enum kbscan */
/* kbscans indices */
enum kbscan {
    Int=    0,          
    Ext,

    Nscans,
};
/*e: enum kbscan */

/*s: struct Kbscan */
struct Kbscan {
    bool ctl;
    bool shift;
    bool caps;
    bool alt;
    bool altgr;
    bool num;
    /*s: [[Kbscan]] other fields */
    bool esc1;
    int esc2;
    /*x: [[Kbscan]] other fields */
    int buttons;
    /*x: [[Kbscan]] other fields */
    bool collecting;
    int nk;
    Rune    kc[5];
    /*e: [[Kbscan]] other fields */
};
/*e: struct Kbscan */


extern Kbscan kbscans[Nscans];

extern void kbdputsc(byte k, int external);
/*e: portkbd.h */
