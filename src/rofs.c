/*
	ROFS file manager

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "file_functions.h"

/*--- Types ---*/

typedef struct {
	Uint32 unknown0[5];
	Uint8 unknown1;
} __attribute__((packed)) rofs_header_t;

/*typedef struct {
	Uint8 *dirname[];
} rofs_dir_level1_t;*/

typedef struct {
	Uint32 offset;
	Uint32 length;
	/*Uint8 *dirname[];*/
} rofs_dir_level2_t;

typedef struct {
	Uint32 offset;
	Uint32 length;
	/*Uint8 *filename[];*/
} rofs_file_header_t;

typedef struct {
	Uint16 offset;
	Uint16 num_keys;
	Uint32 length;
	Uint8 ident;
} rofs_crypt_header_t;

/*--- Variables ---*/

Uint8 rofs_header[4096];

/*--- Function prototypes ---*/

void list_files(const char *filename);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/rofs.dat\n", argv[0]);
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
	const char *dir_level1_name, *dir_level2_name;
	rofs_dir_level2_t dir_level2;
	Uint32 offset, num_files;
	int i;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s\n", filename);
		return;
	}

	/* Read header */
	SDL_RWread(src, rofs_header, 4096, 1);

	/* Level1 directory */
	offset = sizeof(rofs_header_t);
	dir_level1_name = &rofs_header[offset];
	/*printf("level1 dir: %s\n", dir_level1_name);*/

	/* Level2 directory */
	offset += strlen(dir_level1_name)+1;
	memcpy(&dir_level2, &rofs_header[offset], sizeof(rofs_dir_level2_t));
	offset += sizeof(rofs_dir_level2_t);
	dir_level2_name = &rofs_header[offset];
	/*printf("level2 dir: %s\n", dir_level2_name);*/

	offset = SDL_SwapLE32(dir_level2.offset)*8;
	SDL_RWseek(src, offset, RW_SEEK_SET);

	/* Number of files */
	SDL_RWread(src, &num_files, 4, 1);
	num_files = SDL_SwapLE32(num_files);

	/*printf("files: %d\n", num_files);*/

	printf("Offset\t\tLength\t\tName\n");
	for (i=0; i<num_files; i++) {
		rofs_file_header_t file_hdr;
		char filename[512];
		int j;

		/* Read file header */
		SDL_RWread(src, &file_hdr, sizeof(file_hdr), 1);
		file_hdr.length = SDL_SwapLE32(file_hdr.length);
		file_hdr.offset = SDL_SwapLE32(file_hdr.offset) * 8;

		/* Read file name */
		j = 0;
		for (;;) {
			SDL_RWread(src, &filename[j], 1,1);
			if (filename[j] == 0) {
				break;
			}
			j++;
		}

		/*printf(" file %d: %s\n", i, filename);*/
		printf("0x%08x\t0x%08x\t%s/%s/%s\n",
			file_hdr.offset, file_hdr.length,
			dir_level1_name, dir_level2_name, filename);
	}

	SDL_RWclose(src);
}
