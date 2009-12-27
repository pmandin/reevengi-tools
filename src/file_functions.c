/*
	File loader/saver

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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

static char *get_filename_ext(const char *src_filename, const char *new_ext)
{
	int dst_namelength = strlen(src_filename)+1;
	char *dst_filename;
	char *posname, *posext;

	dst_filename = (char *) malloc(dst_namelength);
	if (!dst_filename) {
		fprintf(stderr, "Can not allocate %d bytes\n", dst_namelength);
		return NULL;
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
		strcat(dst_filename, new_ext);
	} else {
		++posext;
		strcpy(posext, new_ext);
	}

	return dst_filename;
}

void save_file(const char *filename, void *buffer, int length)
{
	SDL_RWops *dst;

	dst = SDL_RWFromFile(filename, "wb");
	if (!dst) {
		fprintf(stderr, "Can not create %s for writing\n", filename);
		return;
	}

	SDL_RWwrite(dst, buffer, length, 1);

	SDL_RWclose(dst);
}

void save_bmp(const char *src_filename, SDL_Surface *image)
{
	char *dst_filename;
	
	dst_filename = get_filename_ext(src_filename, ".bmp");
	if (!dst_filename) {
		return;
	}

	printf("Saving to %s\n", dst_filename);
	SDL_SaveBMP(image, dst_filename);

	free(dst_filename);
}

void save_tim(const char *src_filename, Uint8 *buffer, int length)
{
	char *dst_filename;

	dst_filename = get_filename_ext(src_filename, ".tim");
	if (!dst_filename) {
		return;
	}

	printf("Saving to %s\n", dst_filename);
	save_file(dst_filename, buffer, length);

	free(dst_filename);
}

void save_pak(const char *src_filename, Uint8 *buffer, int length)
{
	char *dst_filename;

	dst_filename = get_filename_ext(src_filename, ".pak");
	if (!dst_filename) {
		return;
	}

	printf("Saving to %s\n", dst_filename);
	save_file(dst_filename, buffer, length);

	free(dst_filename);
}
