#include "os.h"

#define rotr32(w, c)	(((w) >> (c)) | ((w) << ( 32 - (c))))

#define load32(p)	(((u32int )( (p)[0] ) <<  0) | (( u32int )( (p)[1] ) <<  8) | \
			(( u32int )( (p)[2] ) << 16) | (( u32int )( (p)[3] ) << 24))

static u32int IV[8] = {
	0x6A09E667UL, 0xBB67AE85UL, 0x3C6EF372UL, 0xA54FF53AUL,
	0x510E527FUL, 0x9B05688CUL, 0x1F83D9ABUL, 0x5BE0CD19UL
};

static u8int sigma[10][16] =
{
	{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
	{ 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 },
	{ 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 },
	{  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 },
	{  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 },
	{  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 },
	{ 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 },
	{ 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 },
	{  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 },
	{ 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 },
};

#define G(r,i,a,b,c,d) \
	do { \
		a = a + b + m[sigma[r][2*i+0]];		\
		d = rotr32(d ^ a, 16);                  \
		c = c + d;                              \
		b = rotr32(b ^ c, 12);                  \
		a = a + b + m[sigma[r][2*i+1]];		\
		d = rotr32(d ^ a, 8);			\
		c = c + d;				\
		b = rotr32(b ^ c, 7);			\
	} while(0)

#define ROUND(r) \
	do { \
		G(r,0,v[ 0],v[ 4],v[ 8],v[12]); \
		G(r,1,v[ 1],v[ 5],v[ 9],v[13]); \
		G(r,2,v[ 2],v[ 6],v[10],v[14]); \
		G(r,3,v[ 3],v[ 7],v[11],v[15]); \
		G(r,4,v[ 0],v[ 5],v[10],v[15]); \
		G(r,5,v[ 1],v[ 6],v[11],v[12]); \
		G(r,6,v[ 2],v[ 7],v[ 8],v[13]); \
		G(r,7,v[ 3],v[ 4],v[ 9],v[14]); \
	} while(0)

void
_blake2sblock(u32int h[16], uchar *b, u32int blen, u64int offset, u32int f)
{
	u32int m[16];
	u32int v[16];
	int i;
	uchar *e;

	for(e = b + blen; b < e; b += 64){
		for(i = 0; i < 16; i++)
			m[i] = load32(b + i*sizeof h[0]);
	
		for(i = 0; i < 8; i++)
			v[i] = h[i];
	
		v[ 8] = IV[0];
		v[ 9] = IV[1];
		v[10] = IV[2];
		v[11] = IV[3];
		v[12] = ((u32int)(offset>> 0)) ^ IV[4];
		v[13] = ((u32int)(offset>>32)) ^ IV[5];
		v[14] = f ^ IV[6];
		v[15] = IV[7];

		ROUND(0);
		ROUND(1);
		ROUND(2);
		ROUND(3);
		ROUND(4);
		ROUND(5);
		ROUND(6);
		ROUND(7);
		ROUND(8);
		ROUND(9);
	
		for(i = 0; i < 8; i++)
			h[i] = h[i] ^ v[i] ^ v[i + 8];

		offset += 64;
	}
}
