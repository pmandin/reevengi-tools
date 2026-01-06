/*
	PRS file depacker

	Copyright (C) 2026	Patrice Mandin

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

/*
Based on original Python version
https://forums.qhimm.com/index.php?topic=11225.0

# SEGA PRS Decompression (LZS variant)
# Credits:
# based on information/comparing output with
# Nemesis/http://www.romhacking.net/utils/671/
# puyotools/http://code.google.com/p/puyotools/
# fuzziqer software prs/http://www.fuzziqersoftware.com/projects.php
 */

#include <SDL.h>

/*--- Defines ---*/

#define CHUNK_SIZE 65536

/*--- Variables ---*/

static Uint8 *dstPointer;
static int dstBufLen;
static int dstOffset;

static int srcPos;
static int srcBit;

static int cmd;

/*--- Functions ---*/

static int prs_read_bit(Uint8 *srcPtr)
{
	int retvalue;

	if (srcBit==0) {
		cmd = srcPtr[srcPos++];
		srcBit = 8;
	}

	retvalue = cmd & 1;
	cmd >>= 1;
	srcBit--;

	return retvalue;
}

static void prs_write_dest(Uint8 value)
{
	if ((dstPointer==NULL) || (dstOffset>=dstBufLen)) {
		dstBufLen += CHUNK_SIZE;
		dstPointer = realloc(dstPointer, dstBufLen);
		if (dstPointer==NULL) {
			fprintf(stderr, "prs: can not allocate %d bytes\n", dstBufLen);
			return;
		}
	}

	dstPointer[dstOffset++] = value;
}

void prs_depack(Uint8 *srcPtr, int srcLen, Uint8 **dstBufPtr, int *dstLength)
{
	*dstBufPtr = dstPointer = NULL;
	*dstLength = dstBufLen = dstOffset = 0;

	srcPos = 0;
	srcBit = 0;
	while (srcPos<srcLen) {
		int localCmd, localT, offset, amount, start, j;

		localCmd = prs_read_bit(srcPtr);
		if (localCmd) {
			prs_write_dest(srcPtr[srcPos++]);
			continue;
		}

		localT = prs_read_bit(srcPtr);
		if (localT) {
			int a,b;

			a = srcPtr[srcPos++];
			b = srcPtr[srcPos++];

			offset = ((b << 8) | a) >> 3;
			amount = a & 7;

			if (srcPos<srcLen) {
				if (amount == 0) {
					amount = srcPtr[srcPos++] + 1;
				} else {
					amount += 2;
				}
			}

			start = dstOffset - 0x2000 + offset;
		} else {
			amount = 0;

			for (j=0; j<2; j++) {
				amount <<= 1;
				amount |= prs_read_bit(srcPtr);
			}
			offset = srcPtr[srcPos++];
			amount += 2;

			start = dstOffset - 0x100 + offset;
		}

		for(j=0; j<amount; j++) {
			if (start < 0) {
				prs_write_dest(0);
			} else if (start < dstOffset) {
				prs_write_dest( dstPointer[start] );
			} else {
				prs_write_dest(0);
			}
			start++;
		}
	}

	/* Return depacked buffer */
	*dstBufPtr = (Uint8 *) dstPointer;
	*dstLength = dstOffset;
}
