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
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "file_functions.h"
#include "depack_bsssld.h"

int depack_image(const char *filename)
{
	SDL_RWops *src;
	Uint8 *srcBuffer, *dstBuffer;
	int dstBufLen;
	int retval = 1;
	Sint64 srcLen;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", filename);
		return retval;
	}

	/* Read whole file in memory */
	srcLen = SDL_RWseek(src, 0, RW_SEEK_END);
	SDL_RWseek(src, 0, RW_SEEK_SET);

	srcBuffer = malloc(srcLen);
	if (!srcBuffer)
		return retval;

	SDL_RWread(src, srcBuffer, srcLen, 1);
	SDL_RWclose(src);

	bsssld_depack(srcBuffer, srcLen, &dstBuffer, &dstBufLen);
	if (dstBuffer && dstBufLen) {
		save_tim(filename, dstBuffer, dstBufLen);
		free(dstBuffer);
	}

	free(srcBuffer);

	return retval;
}

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.bin\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	retval = depack_image(argv[1]);

	SDL_Quit();
	return retval;
}
