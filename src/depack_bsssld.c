/*
	BSS SLD file depacker

	Copyright (C) 2020	Patrice Mandin

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
#include <stdlib.h>
#include <fcntl.h>

#include <SDL.h>

void memcpy_overlap(Uint8 *dest, Uint8 *src, int count) {
	int i;

	for (i=0; i<count; i++) {
		dest[i] = src[i];
	}
}

void bsssld_depack_re2(Uint8 *srcPtr, int srcLen, Uint8 **dstBufPtr, int *dstLength)
{
	Uint32 buflen;
	Uint8 *dstPtr;
	int srcPos, dstPos;
	int count;

	buflen = SDL_SwapLE32(*((Uint32 *)srcPtr));

	dstPtr = *dstBufPtr = malloc(buflen);
	memset(*dstBufPtr, 0, buflen);
	*dstLength = buflen;

	srcPos = 6;
	dstPos = 0;
	while ((srcPos<srcLen) && (dstPos<buflen)) {

		while ((srcPtr[srcPos] & 0x10)==0) {
			count = srcPtr[srcPos] & 0x0f;
			int srcOffset = (-256 | (srcPtr[srcPos] & 0xe0))<<3;
			srcOffset |= srcPtr[srcPos+1];
			if (count == 0x0f) {
				count += srcPtr[srcPos+2];
				srcPos += 3;
			} else {
				srcPos += 2;
			}
			count += 3;

			memcpy_overlap(&dstPtr[dstPos], &dstPtr[dstPos+srcOffset], count);
			dstPos += count;
		}

		if (srcPtr[srcPos] == 0xff)
			break;

		count = ((srcPtr[srcPos++] | 0xffe0) ^ 0xffff)+1;
		if (count == 0x10) {
			count += srcPtr[srcPos++];
		}

		memcpy(&dstPtr[dstPos], &srcPtr[srcPos], count);
		dstPos += count;
		srcPos += count;
	}
}

void bsssld_depack_re3(Uint8 *srcPtr, int srcLen, Uint8 **dstBufPtr, int *dstLength)
{
	Uint32 buflen;
	Uint8 *dstPtr;
	int srcPos, dstPos;
	int count, offset;

	buflen = SDL_SwapLE32(*((Uint32 *)srcPtr));

	dstPtr = *dstBufPtr = malloc(buflen + srcLen);
	memset(*dstBufPtr, 0, buflen);
	*dstLength = buflen;

	srcPos = 4;
	dstPos = 0;
	while ((srcPos<srcLen) /*&& (dstPos<buflen)*/) {
		if (srcPtr[srcPos] == 0) {
			break;
		}

		if ((srcPtr[srcPos] & 0x80) != 0) {
			count = srcPtr[srcPos++] & 0x7f;

			memcpy(&dstPtr[dstPos], &srcPtr[srcPos], count);
			srcPos += count;
			dstPos += count;
		} else {
			offset = srcPtr[srcPos++]<<8;
			offset |= srcPtr[srcPos++];
			count = (offset>>11)+2;
			offset &= 0x7ff;

			memcpy_overlap(&dstPtr[dstPos], &dstPtr[dstPos-(offset+4)], count);
			dstPos += count;
		}
	}
}
