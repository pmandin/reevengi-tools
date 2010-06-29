/*
	PAK file packer

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

#include "pack_pak.h"
#include "file_functions.h"
#include "param.h"
#include "background_tim.h"

/*--- Variables ---*/

/* Flag for image stored with 4 pixels less
   (used for shaking rooms, like lifts or with rolling boulders)
 */
static int remove4pix = 0;	

/*--- Functions prototypes ---*/

int convert_image(const char *filename);
void remove_4_pixels(Uint8 *srcBuffer, int srcBufLen);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s [-r4] /path/to/filename.ext\n", argv[0]);
		return 1;
	}

	if (param_check("-r4",argc,argv)>=0) {
		remove4pix = 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	retval = convert_image(argv[argc-1]);

	SDL_Quit();
	return retval;
}

int convert_image(const char *filename)
{
	SDL_RWops *src;
	Uint8 *dstBuffer, *srcBuffer;
	int dstBufLen, srcBufLen;
	int retval = 1;

	/* Read file in memory */
	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", filename);
		return retval;
	}

	SDL_RWseek(src, 0, RW_SEEK_END);
	srcBufLen = SDL_RWtell(src);
	SDL_RWseek(src, 0, RW_SEEK_SET);

	srcBuffer = (Uint8 *) malloc(srcBufLen);
	if (!srcBuffer) {
		fprintf(stderr, "Can not allocate %d bytes in memory\n", srcBufLen);
		return retval;
	}
	SDL_RWread(src, srcBuffer, srcBufLen, 1);
	SDL_RWclose(src);

	/* Remove 4 pixels ? */
	if (remove4pix) {
		remove_4_pixels(srcBuffer, srcBufLen);
	}

	src = SDL_RWFromMem(srcBuffer, srcBufLen);
	if (src) {
		pak_pack(src, &dstBuffer, &dstBufLen);

		if (dstBuffer && dstBufLen) {
			save_pak(filename, dstBuffer, dstBufLen);

			free(dstBuffer);
			retval = 0;
		} else {
			fprintf(stderr, "Error packing file\n");
		}

		SDL_RWclose(src);
	}

	free(srcBuffer);

	return retval;
}

void remove_4_pixels(Uint8 *srcBuffer, int srcBufLen)
{
	tim_header_t *tim_header = (tim_header_t *) srcBuffer;
	tim_size_t *tim_size;
	Uint32 tim_type, img_offset;
	Uint16 *srcImage, *dstImage;
	int x,y,w,h;

	if (SDL_SwapLE32(tim_header->magic) != MAGIC_TIM) {
		fprintf(stderr, "Not a TIM image\n");
		return;
	}

	if (SDL_SwapLE32(tim_header->type) != TIM_TYPE_16) {
		fprintf(stderr, "Only 16 bpp TIM images can be reparsed\n");
		return;
	}

	img_offset = 16;
	tim_size = (tim_size_t *) (&((Uint8 *) srcBuffer)[img_offset]);
	w = SDL_SwapLE16(tim_size->width);
	h = SDL_SwapLE16(tim_size->height);

	srcImage = dstImage = (Uint16 *) (&((Uint8 *) srcBuffer)[img_offset+sizeof(tim_size_t)]);
	srcImage += 2 + w*2;
	for (y=0; y<h-4; y++) {
		memcpy(dstImage, srcImage, (w-4)*2);
		srcImage += w;
		dstImage += w-4;
	}
}
