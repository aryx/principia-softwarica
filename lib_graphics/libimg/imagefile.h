typedef struct Rawimage Rawimage;

struct Rawimage
{
	Rectangle	r;
	uchar	*cmap;
	int		cmaplen;
	int		nchans;
	uchar	*chans[4];
	int		chandesc;
	int		chanlen;

	int		fields;    	/* defined by format */
	int		gifflags;	/* gif only; graphics control extension flag word */
	int		gifdelay;	/* gif only; graphics control extension delay in cs */
	int		giftrindex;	/* gif only; graphics control extension transparency index */
	int		gifloopcount;	/* number of times to loop in animation; 0 means forever */
};

enum
{
	/* Channel descriptors */
	CRGB	= 0,	/* three channels, no map */
	CYCbCr	= 1,	/* three channels, no map, level-shifted 601 color space */
	CY	= 2,	/* one channel, luminance */
	CRGB1	= 3,	/* one channel, map present */
	CRGBV	= 4,	/* one channel, map is RGBV, understood */
	CRGB24	= 5,	/* one channel in correct data order for loadimage(RGB24) */
	CRGBA32	= 6,	/* one channel in correct data order for loadimage(RGBA32) */
	CYA16	= 7,	/* one channel in correct data order for loadimage(Grey8+Alpha8) */
	CRGBVA16= 8,	/* one channel in correct data order for loadimage(CMAP8+Alpha8) */

	/* GIF flags */
	TRANSP	= 1,
	INPUT	= 2,
	DISPMASK = 7<<2
};


enum{	/* PNG flags */
	II_GAMMA =	1 << 0,
	II_COMMENT =	1 << 1,
};

typedef struct ImageInfo {
	ulong	fields_set;
	double	gamma;
	char	*comment;
} ImageInfo;


Rawimage**	readjpg(int, int);
Rawimage**	readpng(int, int);
Rawimage**	readgif(int, int);
Rawimage**	readpixmap(int, int);

Rawimage**	Breadjpg(Biobuf *b, int);
Rawimage**	Breadpng(Biobuf *b, int);

Rawimage*	torgbv(Rawimage*, int);
Rawimage*	totruecolor(Rawimage*, int);

int		writerawimage(int, Rawimage*);
void*		_remaperror(char*, ...);

typedef struct Memimage Memimage;	/* avoid necessity to include memdraw.h */

char*		writegif(Biobuf*, Image*, char*, int, int);
char*		writeppm(Biobuf*, Image*, char*);

char*		startgif(Biobuf*, Image*, int);
void		endgif(Biobuf*);

Image*		onechan(Image*);
Image*		multichan(Image*);

// memimage variants
char*		memwritegif(Biobuf*, Memimage*, char*, int, int);
char*		memwriteppm(Biobuf*, Memimage*, char*);
char*		memwritepng(Biobuf*, Memimage*, ImageInfo*);

char*		memstartgif(Biobuf*, Memimage*, int);
void		memendgif(Biobuf*);

Memimage*	memonechan(Memimage*);
Memimage*	memmultichan(Memimage*);
