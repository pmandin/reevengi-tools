/*
	BIN file depacker (RE2 PS1 dat/*.bin files)

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

/*--- Types ---*/

typedef struct {
	Uint32 id;
	Uint32 length;
	Uint32 blocks1;
	Uint32 unknown1[1+12];
	Uint8 filename[0x800-0x40];
} bin_header_t;

/*--- Const ---*/

/*--- Variables ---*/

/*--- Function prototypes ---*/

void list_files(const char *filename);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/file.bin\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	list_files(argv[1]);

	SDL_Quit();
	return 0;
}

void list_files(const char *filename)
{
	SDL_RWops *src;
	bin_header_t bin_hdr;
	Uint32 offset, bin_length;
	/*int i;*/

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s\n", filename);
		return;
	}

	SDL_RWseek(src, 0, RW_SEEK_END);
	bin_length = SDL_RWtell(src);
	SDL_RWseek(src, 0, RW_SEEK_SET);

	offset = 0;
	while (offset<bin_length) {
		/*Uint32 fileNum;*/
		int fileLen;

		SDL_RWread(src, &bin_hdr, sizeof(bin_header_t),1);
		if (SDL_SwapLE32(bin_hdr.id) == 0xffffffffUL) {
			break;
		}

		fileLen = SDL_SwapLE32(bin_hdr.length);
		if (fileLen==0) {
			break;
		}

		printf("Offset 0x%08x, Length 0x%08x, %s\n", offset,fileLen,bin_hdr.filename);

		/* Next file */
		offset += SDL_SwapLE32(bin_hdr.blocks1) * 0x800;

		switch(SDL_SwapLE32(bin_hdr.id)) {
			case 1:
				offset -= 0x800;
				break;
			case 5:
			case 9:
				offset += 0x800;
				break;
		}


		SDL_RWseek(src, offset, RW_SEEK_SET);
	}

	SDL_RWclose(src);
}
