/*s: include/cursor.h */
#pragma src "/sys/src/libdraw"

typedef struct	Cursor Cursor;

/*s: struct Cursor */
struct	Cursor
{
    Point	offset;

    byte	clr[2*16];
    byte	set[2*16];
};
/*e: struct Cursor */
/*e: include/cursor.h */
