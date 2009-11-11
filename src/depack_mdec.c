/*
	MDEC depacker

	Copyright (C) 2007	Patrice Mandin
	Copyright (C) 2000	Bero
	Copyright (C) 1997-2000 Psxdev project
		Daniel Balster
		Sergio Moreira
		Andrew Kieschnick
		Kazuki Sakamoto

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <fcntl.h>

#include <SDL.h>

#include "idctfst.h"

/*--- Defines ---*/

#define VLC_ID		0x3800
#define	EOB		0xfe00
#define	DCTSIZE2	64
#define	RUNOF(a)	((a)>>10)
#define	VALOF(a)	((short)((a)<<6)>>6)

#define	ROUND(r)	bs_roundtbl[(r)+256]

#define	SHIFT		12
#define	toFIX(a)	(int)((a)*(1<<SHIFT))
#define	toINT(a)	((a)>>SHIFT)
#define	FIX_1		toFIX(1)
#define	MULR(a)		toINT((a)*toFIX(1.402))
#define	MULG(a)		toINT((a)*toFIX(-0.3437))
#define	MULG2(a)	toINT((a)*toFIX(-0.7143))
#define	MULB(a)		toINT((a)*toFIX(1.772))

enum {B,G,R};

/* this table is based on djpeg by Independent Jpeg Group */

static const int aanscales[DCTSIZE2] = {
	  /* precomputed values scaled up by 14 bits */
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
	  21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
	  19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
	  16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
	  12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
	   8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
	   4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
};

static unsigned char zscan[DCTSIZE2] = {
	0 ,1 ,8 ,16,9 ,2 ,3 ,10,
	17,24,32,25,18,11,4 ,5 ,
	12,19,26,33,40,48,41,34,
	27,20,13,6 ,7 ,14,21,28,
	35,42,49,56,57,50,43,36,
	29,22,15,23,30,37,44,51,
	58,59,52,45,38,31,39,46,
	53,60,61,54,47,55,62,63
};

static unsigned char bs_iqtab[DCTSIZE2] = {
	 2,16,19,22,26,27,29,34,
	16,16,22,24,27,29,34,37,
	19,22,26,27,29,34,34,38,
	22,22,26,27,29,34,37,40,
	22,26,27,29,32,35,40,48,
	26,27,29,32,35,40,48,58,
	26,27,29,34,38,46,56,69,
	27,29,35,38,46,56,69,83
};

/*--- Types ---*/

typedef struct {
	int iqtab[DCTSIZE2];
	SDL_RWops *src;
} bs_context_t;

/*--- Variables ---*/

static Uint16 *dstPointer;
static int dstBufLen;
static int dstOffset;

static Uint8 bs_roundtbl[256*3];

/*--- Functions ---*/

void rl2blk(bs_context_t *ctxt, BLOCK *blk)
{
	SDL_RWops *src = ctxt->src;

	int i,k,q_scale,rl;
	memset(blk,0,6*DCTSIZE2*sizeof(BLOCK));
	for(i=0;i<6;i++) {
		rl = SDL_ReadLE16(src);
		/*printf("%d: 0x%04x, %d\n", i,rl, SDL_RWtell(src));*/
		if (rl==EOB) {
			continue;
		}
		q_scale = RUNOF(rl);
		blk[0] = ctxt->iqtab[0]*VALOF(rl);
		k = 0;
		for(;;) {
			rl = SDL_ReadLE16(src);
			/*printf("    0x%04x, 0x%08x\n", rl, SDL_RWtell(src));*/
			if (rl==EOB) {
				break;
			}
			k += RUNOF(rl)+1;
			blk[zscan[k]] = (ctxt->iqtab[zscan[k]]*q_scale*VALOF(rl))>>3;
		}

		IDCT(blk,k+1);
		
		blk+=DCTSIZE2;
	}
}

static void yuv2rgb24(BLOCK *blk,Uint8 image[][3])
{
	int x,yy;
	BLOCK *yblk = blk+DCTSIZE2*2;
	for(yy=0;yy<16;yy+=2,blk+=4,yblk+=8,image+=8+16) {
		if (yy==8) yblk+=DCTSIZE2;
		for(x=0;x<4;x++,blk++,yblk+=2,image+=2) {
			int r0,b0,g0,y;
			r0 = MULR(blk[DCTSIZE2]); /* cr */
			g0 = MULG(blk[0])+MULG2(blk[DCTSIZE2]);
			b0 = MULB(blk[0]); /* cb */
			y = yblk[0]+128;
			image[0][R] = ROUND(r0+y);
			image[0][G] = ROUND(g0+y);
			image[0][B] = ROUND(b0+y);
			y = yblk[1]+128;
			image[1][R] = ROUND(r0+y);
			image[1][G] = ROUND(g0+y);
			image[1][B] = ROUND(b0+y);
			y = yblk[8]+128;
			image[16][R] = ROUND(r0+y);
			image[16][G] = ROUND(g0+y);
			image[16][B] = ROUND(b0+y);
			y = yblk[9]+128;
			image[17][R] = ROUND(r0+y);
			image[17][G] = ROUND(g0+y);
			image[17][B] = ROUND(b0+y);

			r0 = MULR(blk[4+DCTSIZE2]);
			g0 = MULG(blk[4])+MULG2(blk[4+DCTSIZE2]);
			b0 = MULB(blk[4]);
			y = yblk[DCTSIZE2+0]+128;
			image[8+0][R] = ROUND(r0+y);
			image[8+0][G] = ROUND(g0+y);
			image[8+0][B] = ROUND(b0+y);
			y = yblk[DCTSIZE2+1]+128;
			image[8+1][R] = ROUND(r0+y);
			image[8+1][G] = ROUND(g0+y);
			image[8+1][B] = ROUND(b0+y);
			y = yblk[DCTSIZE2+8]+128;
			image[8+16][R] = ROUND(r0+y);
			image[8+16][G] = ROUND(g0+y);
			image[8+16][B] = ROUND(b0+y);
			y = yblk[DCTSIZE2+9]+128;
			image[8+17][R] = ROUND(r0+y);
			image[8+17][G] = ROUND(g0+y);
			image[8+17][B] = ROUND(b0+y);
		}
	}
}

static void dec_dct_out(bs_context_t *ctxt,Uint16 *image,int size)
{
	BLOCK blk[DCTSIZE2*6];
	int blocksize = (16*16*3)>>1;

	for(;size>0; size-=blocksize>>1,image+=blocksize) {
		rl2blk(ctxt, blk);
		yuv2rgb24(blk, (Uint8 *) image);
	}
}

static void bs_init(void)
{
	int i;
	for(i=0;i<256;i++) {
		bs_roundtbl [i]=0;
		bs_roundtbl [i+256]=i;
		bs_roundtbl [i+512]=255;
	}
}

static void iqtab_init(bs_context_t *ctxt)
{
#define CONST_BITS 14
#define	IFAST_SCALE_BITS 2
	int i;
	for(i=0;i<DCTSIZE2;i++) {
		ctxt->iqtab[i] = bs_iqtab[i]*aanscales[i]>>(CONST_BITS-IFAST_SCALE_BITS);
	}
}

void mdec_depack(SDL_RWops *src, Uint8 **dstBufPtr, int *dstLength,
	int width, int height)
{
	bs_context_t ctxt;
	Uint16	vlc_id;
	int height2 = (height+15)&~15;
	int width2 = (width*3)>>1;
	int w = 8*3;
	int slice = (height2 * w)>>1;
	int x,y;
	Uint16 *image;

	*dstBufPtr = NULL;
	dstOffset = *dstLength = 0;

	ctxt.src = src;

	SDL_RWseek(src, 2, RW_SEEK_CUR); /* skip block length */

	vlc_id = SDL_ReadLE16(src);
	if (vlc_id != VLC_ID) {
		fprintf(stderr, "mdec: Unknown vlc id: 0x%04x\n", vlc_id);
		return;
	}

	image = (Uint16 *) malloc(height2 * w * sizeof(Uint16));
	if (!image) {
		fprintf(stderr, "mdec: Can not allocate memory for temp buffer\n");
		return;
	}

	dstBufLen = width*height*4;
	dstPointer = (Uint16 *) malloc(dstBufLen);
	if (!dstPointer) {
		fprintf(stderr, "mdec: Can not allocate memory for final buffer\n");
		free(image);
		return;
	}
	
	iqtab_init(&ctxt);
	bs_init();

	for (x=0; x<width2; x+=w) {
		Uint16 *dst,*src;
		/*printf("x=%d\n",x);*/

		dec_dct_out(&ctxt,image,slice);

		src = image;
 		/*dst = buf2+x+(0)*width;*/
		dst = &dstPointer[x];
 		for(y=height-1; y>=0; y--) {
 			memcpy(dst,src,w*2);
 			src+=w;
 			dst+=width2;
 		}
	}

	free(image);

	*dstBufPtr = (Uint8 *) dstPointer;
	*dstLength = dstBufLen;
}

SDL_Surface *mdec_surface(Uint8 *source, int width, int height, int row_offset)
{
	SDL_Surface *surface;
	Uint8 *surface_line;
	int x,y;
	Uint32 rmask,gmask,bmask;

	/* Source is BGR */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff << 16;
	gmask = 0xff << 8;
	bmask = 0xff;
#else
	rmask = 0xff;
	gmask = 0xff << 8;
	bmask = 0xff << 16;
#endif

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,24,rmask,gmask,bmask,0);
	if (!surface) {
		return NULL;
	}

	surface_line = surface->pixels;
	if (row_offset<0) {
		int num_pixels = -row_offset>>1;
		surface_line += surface->pitch * num_pixels;
		surface_line += num_pixels*3;
		height -= num_pixels*2;
	}
	for (y=0; y<height; y++) {
		memcpy(surface_line, source, (width+row_offset)*3);
		surface_line += surface->pitch;
		source += (width+row_offset)*3;
	}

	return surface;
}
