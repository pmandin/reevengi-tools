/*
	BSS file depacker

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

#include "depack_vlc.h"
#include "depack_mdec.h"
#include "file_functions.h"

static int bss_w=320;
static int bss_h=240;

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
	vlc_depack(src, &dstBuffer, &dstBufLen);
	SDL_RWclose(src);

	if (dstBuffer && dstBufLen) {
		SDL_RWops *mdec_src;

		mdec_src = SDL_RWFromMem(dstBuffer, dstBufLen);
		if (mdec_src) {
			Uint8 *dstMdecBuf;
			int dstMdecLen;

			mdec_depack(mdec_src, &dstMdecBuf, &dstMdecLen, bss_w, bss_h);
			SDL_RWclose(mdec_src);

			if (dstMdecBuf && dstMdecLen) {
				SDL_Surface *image = mdec_surface(dstMdecBuf, bss_w, bss_h,0);
				if (image) {
					save_bmp(filename, image);

					SDL_FreeSurface(image);

					retval = 0;
				}

				free(dstMdecBuf);
			}
		}
		free(dstBuffer);
	}

	return retval;
}

int main(int argc, char **argv)
{
	int retval, p;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.bss [-w <width>] [-h <height]\n", argv[0]);
		return 1;
	}

	p = param_present("-w", argc, argv);
	if (p && p < argc-1) {
		bss_w = atoi(argv[p+1]);
	}

	p = param_present("-h", argc, argv);
	if (p && p < argc-1) {
		bss_h = atoi(argv[p+1]);
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
