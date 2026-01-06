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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "depack_prs.h"
#include "file_functions.h"
#include "param.h"

/*--- Functions prototypes ---*/

int convert_file(const char *filename);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.xxx\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	retval = convert_file(argv[argc-1]);

	SDL_Quit();
	return retval;
}

int convert_file(const char *filename)
{
	SDL_RWops *src;
	Uint8 *srcBuffer, *dstBuffer;
	int srcLen, dstBufLen;
	int retval = 1;

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

	prs_depack(srcBuffer, srcLen, &dstBuffer, &dstBufLen);

	if (dstBuffer && dstBufLen) {
		char filename_out[512];

		sprintf(filename_out, "%s.out", filename);
		printf("Depacking file %s to %s\n", filename, filename_out);

		save_file(filename_out, dstBuffer, dstBufLen);

		free(dstBuffer);
		retval = 0;
	} else {
		fprintf(stderr, "Error depacking file\n");
	}

	return retval;
}
