/*s: include/cursor.h */
#pragma src "/sys/src/libdraw"

/*s: struct Cursor */
struct	Cursor
{
    Point	offset;
    uchar	clr[2*16];
    uchar	set[2*16];
};
/*e: struct Cursor */
/*e: include/cursor.h */
