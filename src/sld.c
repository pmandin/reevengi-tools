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
#include <string.h>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "file_functions.h"
#include "depack_sld.h"

/*--- Types ---*/

typedef struct {
	Uint32 unknown;
	Uint32 length;
} sld_header_t;

/*--- Const ---*/

/*--- Variables ---*/

/*--- Function prototypes ---*/

void list_files(const char *filename);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/file.sld\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	list_files(argv[1]);

	SDL_Quit();
	return retval;
}

void list_files(const char *filename)
{
	SDL_RWops *src;
	sld_header_t sld_hdr;
	Uint32 offset;
	int i;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s\n", filename);
		return;
	}

	offset = 0;
	i = 0;
	while (SDL_RWread(src, &sld_hdr, sizeof(sld_header_t),1)) {
		int fileLen = SDL_SwapLE32(sld_hdr.length);

		if (fileLen) {
			Uint8 *dstBuffer;
			int dstBufLen;
			char filename_tim[512];

			sprintf(filename_tim, "tim%02x.tim", i);
			printf("Depacking file %d to %s\n", i, filename_tim);

			sld_depack(src, &dstBuffer, &dstBufLen);
			if (dstBuffer && dstBufLen) {
				save_file(filename_tim, dstBuffer, dstBufLen);

				free(dstBuffer);
			}
		} else {
			printf("File %d is empty\n");
			fileLen = 8;
		}

		/* Next file */
		offset += fileLen;
		SDL_RWseek(src, offset, RW_SEEK_SET);
		i++;
	}

	SDL_RWclose(src);
}
