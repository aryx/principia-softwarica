/*s: portkbd.c */
/*
 * keyboard input // the portable part
 */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "../port/portkbd.h"

/*s: global kdebug */
bool kdebug;
/*e: global kdebug */

/*s: enum specialkey */
enum {
    KF=         0xF000,     /* function key (begin Unicode private space) */
    Spec=       0xF800,     /* Unicode private space */


    PF=         Spec|0x20,  /* num pad function key */
    View=       Spec|0x00,  /* view (shift window up) */

    Shift=      Spec|0x60,
    Break=      Spec|0x61,
    Ctrl=       Spec|0x62,
    Alt=        Spec|0x63,
    Caps=       Spec|0x64,
    Num=        Spec|0x65,
    Middle=     Spec|0x66,
    Altgr=      Spec|0x67,

    Kmouse=     Spec|0x100,

    No=         0x00,       /* peter */

    /* KF|1, KF|2, ..., KF|0xC is F1, F2, ..., F12 */

    Home=       KF|13,
    Up=         KF|14,
    Pgup=       KF|15,
    Print=      KF|16,
    Left=       KF|17,
    Right=      KF|18,
    End=        KF|24,
    Down=       View,
    Pgdown=     KF|19,
    Ins=        KF|20,
    Del=        0x7F,
    Scroll=     KF|21,

    Nscan=  128,
};
/*e: enum specialkey */

/*s: global kbtab */
Rune kbtab[Nscan] = 
{
[0x00]  No, 0x1b,   '1',    '2',    '3',    '4',    '5',    '6',
[0x08]  '7',    '8',    '9',    '0',    '-',    '=',    '\b',   '\t',
[0x10]  'q',    'w',    'e',    'r',    't',    'y',    'u',    'i',
[0x18]  'o',    'p',    '[',    ']',    '\n',   Ctrl,   'a',    's',
[0x20]  'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';',
[0x28]  '\'',   '`',    Shift,  '\\',   'z',    'x',    'c',    'v',
[0x30]  'b',    'n',    'm',    ',',    '.',    '/',    Shift,  '*',
[0x38]  Alt,  ' ',    Ctrl,   KF|1,   KF|2,   KF|3,   KF|4,   KF|5,
[0x40]  KF|6,   KF|7,   KF|8,   KF|9,   KF|10,  Num,    Scroll, '7',
[0x48]  '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
[0x50]  '2',    '3',    '0',    '.',    No, No, No, KF|11,
[0x58]  KF|12,  No, No, No, No, No, No, No,
[0x60]  No, No, No, No, No, No, No, No,
[0x68]  No, No, No, No, No, No, No, No,
[0x70]  No, No, No, No, No, No, No, No,
[0x78]  No, No, No, No, No, No, No, No,
};
/*e: global kbtab */

/*s: global kbtabshift */
Rune kbtabshift[Nscan] =
{
[0x00]  No, 0x1b,   '!',    '@',    '#',    '$',    '%',    '^',
[0x08]  '&',    '*',    '(',    ')',    '_',    '+',    '\b',   '\t',
[0x10]  'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',
[0x18]  'O',    'P',    '{',    '}',    '\n',   Ctrl,   'A',    'S',
[0x20]  'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
[0x28]  '"',    '~',    Shift,  '|',    'Z',    'X',    'C',    'V',
[0x30]  'B',    'N',    'M',    '<',    '>',    '?',    Shift,  '*',
[0x38]  Alt,  ' ',    Ctrl,   KF|1,   KF|2,   KF|3,   KF|4,   KF|5,
[0x40]  KF|6,   KF|7,   KF|8,   KF|9,   KF|10,  Num,    Scroll, '7',
[0x48]  '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1',
[0x50]  '2',    '3',    '0',    '.',    No, No, No, KF|11,
[0x58]  KF|12,  No, No, No, No, No, No, No,
[0x60]  No, No, No, No, No, No, No, No,
[0x68]  No, No, No, No, No, No, No, No,
[0x70]  No, No, No, No, No, No, No, No,
[0x78]  No, No, No, No, No, No, No, No,
};
/*e: global kbtabshift */

/*s: global kbtabesc1 */
Rune kbtabesc1[Nscan] =
{
[0x00]  No, No, No, No, No, No, No, No,
[0x08]  No, No, No, No, No, No, No, No,
[0x10]  No, No, No, No, No, No, No, No,
[0x18]  No, No, No, No, '\n',   Ctrl,   No, No,
[0x20]  No, No, No, No, No, No, No, No,
[0x28]  No, No, Shift,  No, No, No, No, No,
[0x30]  No, No, No, No, No, '/',    No, Print,
[0x38]  Altgr,  No, No, No, No, No, No, No,
[0x40]  No, No, No, No, No, No, Break,  Home,
[0x48]  Up, Pgup,   No, Left,   No, Right,  No, End,
[0x50]  Down,   Pgdown, Ins,    Del,    No, No, No, No,
[0x58]  No, No, No, No, No, No, No, No,
[0x60]  No, No, No, No, No, No, No, No,
[0x68]  No, No, No, No, No, No, No, No,
[0x70]  No, No, No, No, No, No, No, No,
[0x78]  No, No, No, No, No, No, No, No,
};
/*e: global kbtabesc1 */

/*s: global kbtabaltgr */
Rune kbtabaltgr[Nscan] =
{
[0x00]  No, No, No, No, No, No, No, No,
[0x08]  No, No, No, No, No, No, No, No,
[0x10]  No, No, No, No, No, No, No, No,
[0x18]  No, No, No, No, '\n',   Ctrl,   No, No,
[0x20]  No, No, No, No, No, No, No, No,
[0x28]  No, No, Shift,  No, No, No, No, No,
[0x30]  No, No, No, No, No, '/',    No, Print,
[0x38]  Altgr,  No, No, No, No, No, No, No,
[0x40]  No, No, No, No, No, No, Break,  Home,
[0x48]  Up, Pgup,   No, Left,   No, Right,  No, End,
[0x50]  Down,   Pgdown, Ins,    Del,    No, No, No, No,
[0x58]  No, No, No, No, No, No, No, No,
[0x60]  No, No, No, No, No, No, No, No,
[0x68]  No, No, No, No, No, No, No, No,
[0x70]  No, No, No, No, No, No, No, No,
[0x78]  No, No, No, No, No, No, No, No,
};
/*e: global kbtabaltgr */

// see kbtab.c for the ctrl one, put in another file because
// of issues with TeX with the special characters it contain
// can't LPize it.
/*s: global kbtabctrl decl */
extern Rune kbtabctrl[];
/*e: global kbtabctrl decl */

extern int mouseshifted;
extern void (*kbdmouse)(int);

// defined in <arch>/kbd.c
extern void setleds(Kbscan *kbscan);

/*s: global kbscans */
// hash<enum<kbscan>, Kbscan>
Kbscan kbscans[Nscans]; /* kernel and external scan code state */
/*e: global kbscans */


// called when write in /dev/kbin (see devkbin.c)
/*s: function kbdputsc */
/*
 * Scan code processing
 */
void
kbdputsc(byte k, int external)
{
    bool keyup;
    Kbscan *kbscan;
    // Rune is (uint) but actually 'c' can get the result of functions
    // like latin1() which can return negative numbers.
    long c = k; 

    if(external)
        kbscan = &kbscans[Ext];
    else
        kbscan = &kbscans[Int];

    /*s: [[kbdputsc()]] debugging */
    if(kdebug)
        print("sc %x (ms %d)\n", k, mouseshifted);
    /*e: [[kbdputsc()]] debugging */

    /*s: [[kbdputsc()]] esc key handling part1 and possible return */
    /*
     *  e0's is the first of a 2 character sequence, e1 the first
     *  of a 3 character sequence (on the safari)
     */
    if(c == 0xe0){
        kbscan->esc1 = true;
        return;
    } else if(c == 0xe1){
        kbscan->esc2 = 2;
        return;
    }
    /*e: [[kbdputsc()]] esc key handling part1 and possible return */

    keyup = c & 0x80; // key released (1 bit)
    c &= 0x7f; // 128

    /*s: [[kbdputsc()]] ensures c is in boundary */
    if(c > sizeof kbtab){
        // how could reach that? kbtab has 0x80 elts and do c&=0x7f.
        c |= keyup;
        if(c != 0xFF)   /* these come fairly often: CAPSLOCK U Y */
            print("unknown key %ux\n", c);
        return;
    }
    /*e: [[kbdputsc()]] ensures c is in boundary */

    /*s: [[kbdputsc()]] esc key handling part2 and possible return */
    if(kbscan->esc1){
        c = kbtabesc1[c];
        kbscan->esc1 = false;
    } else if(kbscan->esc2){
        kbscan->esc2--;
        return;
    }
    /*e: [[kbdputsc()]] esc key handling part2 and possible return */
    else if(kbscan->shift)
        c = kbtabshift[c];
    else if(kbscan->altgr)
        c = kbtabaltgr[c];
    else if(kbscan->ctl)
        c = kbtabctrl[c];
    else
        c = kbtab[c];

    if(kbscan->caps && c<='z' && c>='a')
        c += 'A' - 'a';

    /*
     *  keyup only important for shifts
     */
    if(keyup){
        switch(c){
        case Ctrl:
            kbscan->ctl = false;
            break;
        case Alt:
            kbscan->alt = false;
            break;
        case Altgr:
            kbscan->altgr = false;
            break;
        case Shift:
            kbscan->shift = false;
            /*s: [[kbdputsc()]] reset mouseshift */
            mouseshifted = false;
            /*e: [[kbdputsc()]] reset mouseshift */
            /*s: [[kbdputsc()]] debugging up shift */
            if(kdebug)
                print("shiftclr\n");
            /*e: [[kbdputsc()]] debugging up shift */
            break;
        /*s: [[kbdputsc()]] mouse keyup cases */
        case Kmouse|1:
        case Kmouse|2:
        case Kmouse|3:
        case Kmouse|4:
        case Kmouse|5:
            kbscan->buttons &= ~(1<<(c-Kmouse-1));
            if(kbdmouse)
                kbdmouse(kbscan->buttons);
            break;
        /*e: [[kbdputsc()]] mouse keyup cases */
        }
        return;
    }

    /*
     *  normal character
     */
    if(!(c & (Spec|KF))){
        /*s: [[kbdputsc()]] reboot if ctl-alt-del */
        if(kbscan->ctl)
            if(kbscan->alt && c == Del) // Ctl-Alt-Del
                exit(0);
        /*e: [[kbdputsc()]] reboot if ctl-alt-del */
        /*s: [[kbdputsc()]] if collecting */
        if(kbscan->collecting){
            int i;
            // pad's additional overflow checking, just to make sure
            if(kbscan->nk >= nelem(kbscan->kc)) {
              kbscan->nk = 0;
              kbscan->collecting = false;
              print("collecting overflow, possible bug in latin1()\n");
            }
            kbscan->kc[kbscan->nk++] = c;
            c = latin1(kbscan->kc, kbscan->nk);
            if(c < -1)  /* need more keystrokes */
                return;
            if(c != -1) /* valid sequence */
                kbdputc(c);
            else    /* dump characters */
                for(i=0; i<kbscan->nk; i++)
                    kbdputc(kbscan->kc[i]);
            kbscan->nk = 0;
            kbscan->collecting = false;
        }
        /*e: [[kbdputsc()]] if collecting */
        else {
            kbdputc(c); //!! adding the character in kbd staging area
        }
    }else{
        switch(c){
        case Ctrl:
            kbscan->ctl = true;
            break;
        case Alt:
            kbscan->alt = true;
            /*s: [[kbdputsc()]] start collecting */
            /*
             * VMware and Qemu use Ctl-Alt as the key combination
             * to make the VM give up keyboard and mouse focus.
             * This has the unfortunate side effect that when you
             * come back into focus, Plan 9 thinks you want to type
             * a compose sequence (you just typed alt). 
             *
             * As a clumsy hack around this, we look for ctl-alt
             * and don't treat it as the start of a compose sequence.
             */
            if(!kbscan->ctl){
                kbscan->collecting = true;
                kbscan->nk = 0;
            }
            /*e: [[kbdputsc()]] start collecting */
            break;
        case Altgr:
            kbscan->altgr = true;
            break;
        case Shift:
            kbscan->shift = true;
            /*s: [[kbdputsc()]] set mouseshift */
            mouseshifted = true;
            /*e: [[kbdputsc()]] set mouseshift */
            /*s: [[kbdputsc()]] debugging down shift */
            if(kdebug)
                print("shift\n");
            /*e: [[kbdputsc()]] debugging down shift */
            break;
        case Caps:
            kbscan->caps ^= true;
            break;
        case Num:
            kbscan->num ^= true;
            if(!external)
                setleds(kbscan);
            break;
        /*s: [[kbdputsc()]] mouse keydown cases */
        case Kmouse|1:
        case Kmouse|2:
        case Kmouse|3:
        case Kmouse|4:
        case Kmouse|5:
            kbscan->buttons |= 1<<(c-Kmouse-1);
            if(kbdmouse)
                kbdmouse(kbscan->buttons);
            break;
        /*e: [[kbdputsc()]] mouse keydown cases */
        /*s: [[kbdputsc()]] special keyboard debug keys cases */
        case KF|11:
            print("kbd debug on, F12 turns it off\n");
            kdebug = true;
            kbdputc(c);
            break;
        case KF|12:
            kdebug = false;
            kbdputc(c);
            break;
        /*e: [[kbdputsc()]] special keyboard debug keys cases */
        // e.g. Left, Right or other special key
        default:
            kbdputc(c);
            break;
        }
    }
}
/*e: function kbdputsc */

// called when interact with /dev/kbmap (see devkbmap.c)
/*s: function kbdputmap */
void
kbdputmap(ushort m, ushort scanc, Rune r)
{
    if(scanc >= Nscan)
        error(Ebadarg);
    switch(m) {
    default:
        error(Ebadarg);
    case 0:
        kbtab[scanc] = r;
        break;
    case 1:
        kbtabshift[scanc] = r;
        break;
    case 2:
        kbtabesc1[scanc] = r;
        break;
    case 3:
        kbtabaltgr[scanc] = r;
        break;
    case 4: 
        kbtabctrl[scanc] = r;
        break;
    }
}
/*e: function kbdputmap */

/*s: function kbdgetmap */
int
kbdgetmap(uint offset, int *t, int *sc, Rune *r)
{
    if ((int)offset < 0)
        error(Ebadarg);
    *t = offset/Nscan;
    *sc = offset%Nscan;
    switch(*t) {
    default:
        return 0;
    case 0:
        *r = kbtab[*sc];
        return 1;
    case 1:
        *r = kbtabshift[*sc];
        return 1;
    case 2:
        *r = kbtabesc1[*sc];
        return 1;
    case 3:
        *r = kbtabaltgr[*sc];
        return 1;
    case 4:
        *r = kbtabctrl[*sc];
        return 1;
    }
}
/*e: function kbdgetmap */
/*e: portkbd.c */
