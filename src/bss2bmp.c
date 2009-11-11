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

#include "file_functions.h"

void save_bmp(const char *src_filename, SDL_Surface *image)
{
	SDL_RWops *dst;
	int dst_namelength = strlen(src_filename)+1;
	char *dst_filename;
	char *posname, *posext;

	dst_filename = (char *) malloc(dst_namelength);
	if (!dst_filename) {
		fprintf(stderr, "Can not allocate %d bytes\n", dst_namelength);
		return;
	}

	posname = strrchr(src_filename, '/');
	if (posname) {
		++posname;	/* Go after / */
	} else {
		posname = strrchr(src_filename, '\\');
		if (posname) {
			++posname;	/* Go after \\ */
		} else {
			/* No directory in source filename */
			posname = (char *) src_filename;
		}
	}
	sprintf(dst_filename, "%s", posname);

	posext = strrchr(dst_filename, '.');
	if (!posext) {
		strcat(dst_filename, ".bmp");
	} else {
		++posext;
		strcpy(posext, "bmp");
	}

	SDL_SaveBMP(image, dst_filename);

	free(dst_filename);
}

int main(int argc, char **argv)
{
	SDL_RWops *src;
	SDL_RWops *mdec_src;
	Uint8 *dstBuffer;
	int dstBufLen;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.bss\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	src = SDL_RWFromFile(argv[1], "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", argv[1]);
		return 1;
	}
	vlc_depack(src, &dstBuffer, &dstBufLen);
	SDL_FreeRW(src);
			
	if (dstBuffer && dstBufLen) {
		SDL_RWops *mdec_src;

		mdec_src = SDL_RWFromMem(dstBuffer, dstBufLen);
		if (mdec_src) {
			Uint8 *dstMdecBuf;
			int dstMdecLen;

			mdec_depack(mdec_src, &dstMdecBuf, &dstMdecLen, 320,240);
			SDL_FreeRW(mdec_src);

			if (dstMdecBuf && dstMdecLen) {
				SDL_Surface *image = mdec_surface(dstMdecBuf,320,240,0);
				if (image) {
					save_bmp(argv[1], image);

					SDL_FreeSurface(image);
				}

				free(dstMdecBuf);
			}
		}
		free(dstBuffer);
	}

	return 0;
}
