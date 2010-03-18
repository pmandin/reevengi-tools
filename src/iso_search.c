/*
	ISO image file search

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

/*--- Defines ---*/

#define MAGIC_TIM	0x10
#define TIM_TYPE_4	8
#define TIM_TYPE_8	9
#define TIM_TYPE_16	2

/*--- Functions prototypes ---*/

int browse_iso(const char *filename);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.iso\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	retval = browse_iso(argv[1]);

	SDL_Quit();
	return retval;
}

int browse_iso(const char *filename)
{
	SDL_RWops *src;
	Uint32 length, blocks, offset;
	Uint8 data[2352];
	int i;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", filename);
		return 1;
	}

	SDL_RWseek(src, 0, RW_SEEK_END);
	length = SDL_RWtell(src);
	SDL_RWseek(src, 0, RW_SEEK_SET);

	blocks = length/2352;
	if (blocks*2352 != length) {
		fprintf(stderr, "Image must have size multiple of 2352 bytes\n");
		SDL_RWclose(src);
		return 1;
	}

	for (i=0; i<blocks; i++) {
		Uint32 value;

		offset = i * 2352;
		SDL_RWseek(src, offset, RW_SEEK_SET);
		SDL_RWread(src, data, 2352, 1);

		value = (data[24+3]<<24)|
			(data[24+2]<<16)|
			(data[24+1]<<8)|
			data[24];

		if (value == MAGIC_TIM) {
			/* TIM image ? */
			value = (data[28+3]<<24)|
				(data[28+2]<<16)|
				(data[28+1]<<8)|
				data[28];

			switch(value) {
				case TIM_TYPE_4:
					printf("Sector %d (offset 0x%08x): 4 bits TIM image\n",i,offset);
					break;
				case TIM_TYPE_8:
					printf("Sector %d (offset 0x%08x): 8 bits TIM image\n",i,offset);
					break;
				case TIM_TYPE_16:
					printf("Sector %d (offset 0x%08x): 16 bits TIM image\n",i,offset);
					break;
			}
		}
	}

	SDL_RWclose(src);
	return 0;
}
