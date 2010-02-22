/*
	SLD file depacker

	Copyright (C) 2010	Patrice Mandin

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

/*--- Functions ---*/

void sld_depack(SDL_RWops *src, Uint8 **dstBufPtr, int *dstLength)
{
	Uint32 numblocks, buflen = 65536;
	Uint8 start, *dst;
	int i, j, count, offset, dstIndex=0;

	*dstBufPtr = NULL;
	*dstLength = 0;

	SDL_RWread(src, &numblocks, sizeof(Uint32), 1);
	numblocks = SDL_SwapLE32(numblocks);
	if (numblocks==0) {
		return;
	}

	dst = malloc(buflen);

	for (i=0; i<numblocks; i++) {
		SDL_RWread(src, &start, sizeof(Uint8), 1);
		if (start & 0x80) {
			count = start & 0x7f;

			if (dstIndex+count>buflen) {
				buflen += 65536;
				dst = realloc(dst, buflen);
			}

			SDL_RWread(src, &dst[dstIndex], count, 1);
			dstIndex += count;
		} else {
			Uint32 tmp = start<<8;

			SDL_RWread(src, &start, sizeof(Uint8), 1);
			tmp |= start;

			offset = (tmp & 0x7ff)+4;
			count = (tmp>>11)+2;

			if (dstIndex+count>buflen) {
				buflen += 65536;
				dst = realloc(dst, buflen);
			}

			for (j=0; j<count; j++) {
				dst[dstIndex+j] = dst[dstIndex-offset+j];
			}
			dstIndex += count;
		}
	}

	*dstBufPtr = realloc(dst, dstIndex);
	*dstLength = dstIndex;
}
