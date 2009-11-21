/*
	PTC file converter

	Copyright (C) 2009	Patrice Mandin

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

int convert_image(const char *filename)
{
	SDL_RWops *src;
	SDL_Surface *image;
	Uint8 *dstBuffer;
	int dstBufLen;
	int width = 0, height = 0;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", filename);
		return 1;
	}
	dstBufLen=SDL_RWseek(src, 0, SEEK_END);
	SDL_RWseek(src, 0, SEEK_SET);

	dstBuffer = (Uint8 *) malloc(dstBufLen);
	if (dstBuffer) {
		SDL_RWread(src, dstBuffer, dstBufLen, 1);
	}
	SDL_RWclose(src);

	if ((dstBuffer==NULL) || (dstBufLen==0)) {
		fprintf(stderr, "Error loading file\n");
		return 1;
	}

	switch(dstBufLen) {
		case 548352:
			width=224;
			height=816;
			break;
		default:
			break;
	}

	if ((width==0) || (height==0)) {
		fprintf(stderr, "Unknown dimensions for length %d\n", dstBufLen);
		free(dstBuffer);
		return 1;
	}

	image = SDL_CreateRGBSurfaceFrom(dstBuffer, width, height, 24, width*3,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		255<<16,255<<8,255
#else
		255,255<<8,255<<16
#endif
		,0);
	if (image) {
		save_bmp(filename, image);

		SDL_FreeSurface(image);
	}

	free(dstBuffer);
	return 0;
}

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.ptc\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	retval = convert_image(argv[1]);

	SDL_Quit();
	return retval;
}
