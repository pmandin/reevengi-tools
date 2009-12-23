/*
	ADT file depacker

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

#include "depack_adt.h"
#include "file_functions.h"

int convert_image(const char *filename)
{
	SDL_RWops *src;
	Uint8 *dstBuffer;
	int dstBufLen;
	int retval = 1;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", filename);
		return retval;
	}
	adt_depack(src, &dstBuffer, &dstBufLen);
	SDL_RWclose(src);

	if (dstBuffer && dstBufLen) {
		if (dstBufLen == 320*256*2) {
			/* Raw image, save as BMP */
			SDL_Surface *image = adt_surface((Uint16 *) dstBuffer, 1);
			if (image) {
				save_bmp(filename, image);
				SDL_FreeSurface(image);

				retval = 0;
			}
		} else {
			/* Tim image */
			save_tim(filename, dstBuffer, dstBufLen);

			retval = 0;
		}

		free(dstBuffer);
	} else {
		fprintf(stderr, "Error depacking file\n");
	}

	return retval;
}

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.adt\n", argv[0]);
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
