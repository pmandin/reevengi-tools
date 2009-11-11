/*
	PAK file depacker

	Copyright (C) 2007	Patrice Mandin
	Copyright (C) 2006	The ScummVM project
	Copyright (C) 1999-2001	Sarien Team

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

/*--- Defines ---*/

#define CHUNK_SIZE 32768

#define DECODE_SIZE 35024

/*--- Types ---*/

typedef struct {
	long flag;
	long index;
	long value;	
} re1_pack_t;

/*--- Variables ---*/

static Uint8 *dstPointer;
static int dstBufLen;
static int dstOffset;

static unsigned char srcByte;
static int tmpMask;

static re1_pack_t tmpArray2[DECODE_SIZE];
static unsigned char decodeStack[DECODE_SIZE];

/*--- Functions ---*/

static int pak_read_bits(SDL_RWops *src, int num_bits)
{
	unsigned long value=0, mask;

	mask = 1<<(--num_bits);

	while (mask>0) {
		if (tmpMask == 0x80) {
			if ( !SDL_RWread( src, &srcByte, 1, 1 ) ) {
				srcByte = 0;
			}
			/*srcByte = srcPointer[srcOffset++];*/
		}

		if ((tmpMask & srcByte)!=0) {
			value |= mask;
		}

		tmpMask >>= 1;
		mask >>= 1;

		if (tmpMask == 0) {
			tmpMask = 0x80;
		}
	}

	return value;
}

static int pak_decodeString(int decodeStackOffset, unsigned long code)
{
	while (code>255) {
		decodeStack[decodeStackOffset++] = tmpArray2[code].value;
		code = tmpArray2[code].index;
	}
	decodeStack[decodeStackOffset] = code;

	return decodeStackOffset;
}

static void pak_write_dest(Uint8 value)
{
	if ((dstPointer==NULL) || (dstOffset>=dstBufLen)) {
		dstBufLen += CHUNK_SIZE;
		dstPointer = realloc(dstPointer, dstBufLen);
		if (dstPointer==NULL) {
			fprintf(stderr, "pak: can not allocate %d bytes\n", dstBufLen);
			return;
		}
	}

	dstPointer[dstOffset++] = value;
}

void pak_depack(SDL_RWops *src, Uint8 **dstBufPtr, int *dstLength)
{
	int num_bits_to_read, i;
	int lzwnew, c, lzwold, lzwnext;
	int stop = 0;

	*dstBufPtr = dstPointer = NULL;
	*dstLength = dstBufLen = dstOffset = 0;

	tmpMask = 0x80;
	srcByte = 0;	

	memset(tmpArray2, 0, sizeof(tmpArray2));

	while (!stop) {
		for (i=0; i<DECODE_SIZE; i++) {
			tmpArray2[i].flag = 0xffffffff;
		}
		lzwnext = 0x103;
		num_bits_to_read = 9;

		c = lzwold = pak_read_bits(src, num_bits_to_read);

		if (lzwold == 0x100) {
			break;
		}

		pak_write_dest(c);

		for(;;) {
			lzwnew = pak_read_bits(src, num_bits_to_read);

			if (lzwnew == 0x100) {
				stop = 1;
				break;
			}

			if (lzwnew == 0x102) {
				break;
			}

			if (lzwnew == 0x101) {
				num_bits_to_read++;
				continue;
			}

			if (lzwnew >= lzwnext) {
				decodeStack[0] = c;
				i = pak_decodeString(1, lzwold);
			} else {
				i = pak_decodeString(0, lzwnew);
			}	

			c = decodeStack[i];

			while (i>=0) {
				pak_write_dest(decodeStack[i--]);
			}

			tmpArray2[lzwnext].index = lzwold;
			tmpArray2[lzwnext].value = c;
			lzwnext++;

			lzwold = lzwnew;
		}
	}

	/* Return depacked buffer */
	*dstBufPtr = (Uint8 *) dstPointer;
	*dstLength = dstOffset;
}
