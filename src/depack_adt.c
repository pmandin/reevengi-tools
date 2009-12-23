/*
	ADT file depacker

	Copyright (C) 2007	Patrice Mandin

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

static Uint8 *dstPointer;
static int dstBufLen;
static int dstOffset;

static Uint8 *srcPointer;
static int srcOffset;
static unsigned char srcByte;
static int srcNumBit;

static Uint8 *tmp32k, *tmp16k;
static int tmp32kOffset, tmp16kOffset;

/* Unpack structure */

typedef struct {
	long start;
	long length;
} unpackArray8_t;

typedef struct {
	long nodes[2];
} node_t;

#define NODE_LEFT 0
#define NODE_RIGHT 1

typedef struct {
	unsigned long start;
	unsigned long length;
	unsigned long *ptr4;
	unpackArray8_t *ptr8;
	node_t *tree;
} unpackArray_t;

static unpackArray_t array1, array2, array3;

static unsigned short freqArray[17];

static void initTmpArray(unpackArray_t *array, int start, int length)
{
	array->start = start;

	array->length = length;

	array->tree = (node_t *) &tmp32k[tmp32kOffset];
	tmp32kOffset += length * 2 * sizeof(node_t);

	array->ptr8 = (unpackArray8_t *) &tmp32k[tmp32kOffset];
	tmp32kOffset += length * sizeof(unpackArray8_t);

	array->ptr4 = (unsigned long *) &tmp32k[tmp32kOffset];
	tmp32kOffset += length * sizeof(unsigned long);
}

static void initTmpArrayData(unpackArray_t *array)
{
	int i;
		
	for (i=0; i<array->length; i++) {
		array->ptr4[i] =
		array->ptr8[i].start =
		array->ptr8[i].length =
		array->tree[i].nodes[NODE_LEFT] =
		array->tree[i].nodes[NODE_RIGHT] = -1;
	}

	while (i < array->length<<1) {
		array->tree[i].nodes[NODE_LEFT] =
		array->tree[i].nodes[NODE_RIGHT] = -1;
		i++;
	}
}

static int readSrcBits(SDL_RWops *src, int numBits)
{
	int orMask = 0, andMask;
	int finalValue;

	finalValue = srcByte;

	while (numBits > srcNumBit) {
		numBits -= srcNumBit;
		andMask = (1<<srcNumBit)-1;
		andMask &= finalValue;
		andMask <<= numBits;
		if ( !SDL_RWread( src, &srcByte, 1, 1 ) ) {
			srcByte = 0;
		}
		finalValue = srcByte;
		srcNumBit = 8;
		orMask |= andMask;
	}

	srcNumBit -= numBits;
	finalValue >>= srcNumBit;
	finalValue = (finalValue & ((1<<numBits)-1)) | orMask;
	return finalValue;
}

static int readSrcOneBit(SDL_RWops *src)
{
	srcNumBit--;
	if (srcNumBit<0) {
		srcNumBit = 7;
		if ( !SDL_RWread( src, &srcByte, 1, 1 ) ) {
			srcByte = 0;
		}
	}

	return (srcByte>> srcNumBit) & 1;
}

static int readSrcBitfieldArray(SDL_RWops *src, unpackArray_t *array, int curIndex)
{
	do {
		if (readSrcOneBit(src)) {
			curIndex = array->tree[curIndex].nodes[NODE_RIGHT];
		} else {
			curIndex = array->tree[curIndex].nodes[NODE_LEFT];
		}
	} while (curIndex >= array->length);

	return curIndex;
}

static int readSrcBitfield(SDL_RWops *src)
{
	int numZeroBits = 0;
	int bitfieldValue = 1;

	while (readSrcOneBit(src)==0) {
		numZeroBits++;
	}

	while (numZeroBits>0) {
		bitfieldValue = readSrcOneBit(src) + (bitfieldValue<<1);
		numZeroBits--;
	}

	return bitfieldValue;
}

static void initUnpackBlockArray(unpackArray_t *array)
{
	unsigned short tmp[18];
	int i, j;

	memset(tmp, 0, sizeof(tmp));

	for (i=0; i<16; i++) {
		tmp[i+2] = (tmp[i+1] + freqArray[i+1])<<1;
	}

	for (i=0;i<18;i++) {
		/*int startTmp = tmp[i];*/
		for (j=0; j<array->length; j++) {
			if (array->ptr8[j].length == i) {
				array->ptr8[j].start = tmp[i]++ & 0xffff;
			}
		}
	}
}

static int initUnpackBlockArray2(unpackArray_t *array)
{
	int i, j;
	int curLength = array->length;
	int curArrayIndex = curLength + 1;

	array->tree[curLength].nodes[NODE_LEFT] =
	array->tree[curLength].nodes[NODE_RIGHT] =
	array->tree[curArrayIndex].nodes[NODE_LEFT] =
	array->tree[curArrayIndex].nodes[NODE_RIGHT] = -1;

	for (i=0; i<array->length; i++) {
		int curPtr8Start = array->ptr8[i].start;
		int curPtr8Length = array->ptr8[i].length;

		curLength = array->length;

		for (j=0; j<curPtr8Length; j++) {
			int curMask = 1<<(curPtr8Length-j-1);
			int arrayOffset;

			if ((curMask & curPtr8Start)!=0) {
				arrayOffset = NODE_RIGHT;
			} else {
				arrayOffset = NODE_LEFT;
			}

			if (j+1 == curPtr8Length) {
				array->tree[curLength].nodes[arrayOffset] = i;
				break;
			}

			if (array->tree[curLength].nodes[arrayOffset] == -1) {
				array->tree[curLength].nodes[arrayOffset] = curArrayIndex;
				array->tree[curArrayIndex].nodes[NODE_LEFT] =
				array->tree[curArrayIndex].nodes[NODE_RIGHT] = -1;
				curLength = curArrayIndex++;
			} else {
				curLength = array->tree[curLength].nodes[arrayOffset];
			}
		}
	}

	return array->length;
}

static void initUnpackBlock(SDL_RWops *src)
{
	int i, j, prevValue, curBit, curBitfield;
	int numValues;
	unsigned short tmp[512];
	unsigned long tmpBufLen;

	/* Initialize array 1 to unpack block */

	prevValue = 0;
	for (i=0; i<array1.length; i++) {
		if (readSrcOneBit(src)) {
			array1.ptr8[i].length = readSrcBitfield(src) ^ prevValue;
		} else {
			array1.ptr8[i].length = prevValue;
		}
		prevValue = array1.ptr8[i].length;
	}

	/* Count frequency of values in array 1 */
	memset(freqArray, 0, sizeof(freqArray));

	for (i=0; i<array1.length; i++) {
		numValues = array1.ptr8[i].length;
		if (numValues <= 16) {
			freqArray[numValues]++;
		}
	}

	initUnpackBlockArray(&array1);
	tmpBufLen = initUnpackBlockArray2(&array1);

	/* Initialize array 2 to unpack block */

	if (array2.length>0) {
		memset(tmp, 0, array2.length);
	}

	curBit = readSrcOneBit(src);
	j = 0;
	while (j < array2.length) {
		if (curBit) {
			curBitfield = readSrcBitfield(src);
			for (i=0; i<curBitfield; i++) {
				tmp[j+i] = readSrcBitfieldArray(src, &array1, tmpBufLen);
			}
			j += curBitfield;
			curBit = 0;
			continue;
		}

		curBitfield = readSrcBitfield(src);
		if (curBitfield>0) {
			memset(&tmp[j], 0, curBitfield*sizeof(unsigned short));
			j += curBitfield;
		}
		curBit = 1;
	}

	j = 0;
	for (i=0; i<array2.length; i++) {
		j = j ^ tmp[i];
		array2.ptr8[i].length = j;
	}

	/* Count frequency of values in array 2 */
	memset(freqArray, 0, sizeof(freqArray));

	for (i=0; i<array2.length; i++) {
		numValues = array2.ptr8[i].length;
		if (numValues <= 16) {
			freqArray[numValues]++;
		}
	}

	initUnpackBlockArray(&array2);

	/* Initialize array 3 to unpack block */

	prevValue = 0;
	for (i=0; i<array3.length; i++) {
		if (readSrcOneBit(src)) {
			array3.ptr8[i].length = readSrcBitfield(src) ^ prevValue;
		} else {
			array3.ptr8[i].length = prevValue;
		}
		prevValue = array3.ptr8[i].length;
	}

	/* Count frequency of values in array 3 */
	memset(freqArray, 0, sizeof(freqArray));

	for (i=0; i<array3.length; i++) {
		numValues = array3.ptr8[i].length;
		if (numValues <= 16) {
			freqArray[numValues]++;
		}
	}

	initUnpackBlockArray(&array3);
}

/* Initialize temporary tables, read each block and depack it */
void adt_depack(SDL_RWops *src, Uint8 **dstBufPtr, int *dstLength)
{
	int blockLength;

	srcPointer = dstPointer = *dstBufPtr = NULL;
	srcByte = srcNumBit = srcOffset = dstOffset = dstBufLen =
		*dstLength = tmp32kOffset = tmp16kOffset = 0;

	tmp32k = (Uint8 *) malloc(4096 * sizeof(unsigned long));
	if (tmp32k == NULL) {
		return;
	}

	tmp16k = (Uint8 *) malloc(16384);
	if (tmp16k == NULL) {
		free(tmp32k);
		return;
	}

	SDL_RWseek(src, 4, RW_SEEK_CUR);

	initTmpArray(&array1, 8, 16);
	initTmpArray(&array2, 8, 512);
	initTmpArray(&array3, 8, 16);

	initTmpArrayData(&array1);
	initTmpArrayData(&array2);
	initTmpArrayData(&array3);

	memset(tmp16k, 0, sizeof(tmp16k));

	blockLength = readSrcBits(src, 8);
	blockLength |= readSrcBits(src, 8)<<8;
	while (blockLength>0) {
		int tmpBufLen, tmpBufLen1, curBlockLength;

		initUnpackBlock(src);

		tmpBufLen = initUnpackBlockArray2(&array2);
		tmpBufLen1 = initUnpackBlockArray2(&array3);

		curBlockLength = 0;		
		while (curBlockLength < blockLength) {
			int curBitfield = readSrcBitfieldArray(src, &array2, tmpBufLen);

			if (curBitfield < 256) {
				/* Realloc if needed */
				if (dstOffset+1 > dstBufLen) {
					dstBufLen += 0x8000;
					dstPointer = realloc(dstPointer, dstBufLen);
				}

				dstPointer[dstOffset++] =
					tmp16k[tmp16kOffset++] = curBitfield;
				tmp16kOffset &= 0x3fff;
			} else {
				int i;
				int numValues = curBitfield - 0xfd;
				int startOffset;
				curBitfield = readSrcBitfieldArray(src, &array3, tmpBufLen1);
				if (curBitfield != 0) {
					int numBits = curBitfield-1;
					curBitfield = readSrcBits(src, numBits) & 0xffff;
					curBitfield += 1<<numBits;
				}

				/* Realloc if needed */
				if (dstOffset+numValues > dstBufLen) {
					dstBufLen += 0x8000;
					dstPointer = realloc(dstPointer, dstBufLen);
				}

				startOffset = (tmp16kOffset-curBitfield-1) & 0x3fff;
				for (i=0; i<numValues; i++) {
					dstPointer[dstOffset++] = tmp16k[tmp16kOffset++] = 
						tmp16k[startOffset++];
					startOffset &= 0x3fff;
					tmp16kOffset &= 0x3fff;
				}
			}

			curBlockLength++;
		}

		blockLength = readSrcBits(src, 8);
		blockLength |= readSrcBits(src, 8)<<8;
	}

	free(tmp16k);
	free(tmp32k);

	*dstLength = dstBufLen;
	*dstBufPtr = dstPointer;
}

SDL_Surface *adt_surface(Uint16 *source, int reorganize)
{
	SDL_Surface *surface;
	Uint16 *surface_line, *src_line;
	int x,y;

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE,320,240,16,31,31<<5,31<<10,0);
	if (!surface) {
		return NULL;
	}

	if (reorganize) {
		/* First 256x256 block */
		surface_line = (Uint16 *) surface->pixels;
		src_line = source;
		for (y=0; y<240; y++) {
			Uint16 *surface_col = surface_line;
			for (x=0; x<256; x++) {
				Uint16 col = *src_line++;
				*surface_col++ = SDL_SwapLE16(col);
			}
			surface_line += 320;
		}

		/* Then 2x64x128 inside 128x128 */
		surface_line = (Uint16 *) surface->pixels;
		surface_line += 256;
		src_line = &source[256*256];
		for (y=0; y<128; y++) {
			Uint16 *surface_col = surface_line;
			Uint16 *src_col = src_line;
			for (x=0; x<64; x++) {
				Uint16 col = *src_col++;
				*surface_col++ = SDL_SwapLE16(col);
			}

			surface_line += 320;
			src_line += 128;
		}

		surface_line = (Uint16 *) surface->pixels;
		surface_line += (320*128)+256;
		src_line = &source[256*256+64];
		for (y=0; y<240-128; y++) {
			Uint16 *surface_col = surface_line;
			Uint16 *src_col = src_line;
			for (x=0; x<64; x++) {
				Uint16 col = *src_col++;
				*surface_col++ = SDL_SwapLE16(col);
			}

			surface_line += 320;
			src_line += 128;
		}
	} else {
		surface_line = (Uint16 *) surface->pixels;
		src_line = source;
		for (y=0; y<240; y++) {
			Uint16 *surface_col = surface_line;
			for (x=0; x<320; x++) {
				Uint16 col = *src_line++;
				*surface_col++ = SDL_SwapLE16(col);
			}
			surface_line += surface->pitch>>1;
		}
	}

	return surface;
}
