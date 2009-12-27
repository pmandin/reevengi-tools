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

/*--- Defines ---*/

#define MAGIC_TIM	0x10
#define TIM_TYPE_4	8
#define TIM_TYPE_8	9
#define TIM_TYPE_16	2

#define ADT_DEPACKED_RAW 0	/* raw 16 bits image, saved as bmp */
#define ADT_DEPACKED_TIM 1	/* tim image, saved as is */
#define ADT_DEPACKED_UNK 2	/* other type, saved as raw */

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
		int img_type = ADT_DEPACKED_UNK;

		if (dstBufLen == 320*256*2) {
			Uint32 *tmp = (Uint32 *) dstBuffer;
			Uint32 offset = SDL_SwapLE32(tmp[0]);

			if (offset<dstBufLen) {
				/* Search header for TIM image */
				offset >>= 2;
				if (SDL_SwapLE32(tmp[offset]) != MAGIC_TIM) {
					img_type = ADT_DEPACKED_RAW;
				} else if ((SDL_SwapLE32(tmp[offset+1]) != TIM_TYPE_4)
					&& (SDL_SwapLE32(tmp[offset+1]) != TIM_TYPE_8)
					&& (SDL_SwapLE32(tmp[offset+1]) != TIM_TYPE_16))
				{
					img_type = ADT_DEPACKED_RAW;
				}
			}
		} else {
			img_type = ADT_DEPACKED_TIM;
		}

		switch(img_type) {
			case ADT_DEPACKED_RAW:
				{
					/* Raw image, save as BMP */
					SDL_Surface *image = adt_surface((Uint16 *) dstBuffer, 1);
					if (image) {
						save_bmp(filename, image);
						SDL_FreeSurface(image);

						retval = 0;
					}
				}
				break;
			case ADT_DEPACKED_TIM:
				{
					/* Tim image */
					save_tim(filename, dstBuffer, dstBufLen);

					retval = 0;
				}
				break;
			case ADT_DEPACKED_UNK:
				{
					/* Unknown, save raw data */
					save_raw(filename, dstBuffer, dstBufLen);

					retval = 0;
				}
				break;
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
