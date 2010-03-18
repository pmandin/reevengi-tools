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

/*#define EXTRACT_FILES	1*/

#define MAGIC_TIM	0x10
#define TIM_TYPE_4	8
#define TIM_TYPE_8	9
#define TIM_TYPE_16	2

#define FILE_TIM	0
#define FILE_EMD	1

#define DATA_LENGTH 2048

/*--- Functions prototypes ---*/

int browse_iso(const char *filename);
void extract_file(SDL_RWops *src, Uint32 start, Uint32 end, int file_type);

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
	int i, extract_flag = 0, file_type = FILE_TIM, new_file_type = FILE_TIM;
	Uint32 start=0,end=0;

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

	start = end = 0;
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
					end = i;
					extract_flag = 1;
					new_file_type = FILE_TIM;
					break;
				case TIM_TYPE_8:
					printf("Sector %d (offset 0x%08x): 8 bits TIM image\n",i,offset);
					end = i;
					extract_flag = 1;
					new_file_type = FILE_TIM;
					break;
				case TIM_TYPE_16:
					printf("Sector %d (offset 0x%08x): 16 bits TIM image\n",i,offset);
					end = i;
					extract_flag = 1;
					new_file_type = FILE_TIM;
					break;
			}
		} else if (value < 512<<10) {
			/* EMD model ? */
			value = (data[28+3]<<24)|
				(data[28+2]<<16)|
				(data[28+1]<<8)|
				data[28];

			if (value==0x0f) {
				printf("Sector %d (offset 0x%08x): EMD model (maybe)\n",i,offset);
				end = i;
				extract_flag = 1;
				new_file_type = FILE_EMD;
			}
		}

		if ((start>16000) && (end!=0) && extract_flag) {
#ifdef EXTRACT_FILES
			extract_file(src, start,end, file_type);
#endif
			extract_flag = 0;
			file_type = new_file_type;
		}
		if (end!=0) {
			start = end;
			end = 0;
		}
	}

	SDL_RWclose(src);
	return 0;
}

void extract_file(SDL_RWops *src, Uint32 start, Uint32 end, int file_type)
{
	Uint8 *buffer;
	Uint32 length = DATA_LENGTH * (end-start);
	int i;
	char filename[16];
	char *fileext = "%08x.bin";
	SDL_RWops *dst;

	buffer = (Uint8 *) malloc(length);
	if (!buffer) {
		return;
	}

	for (i=0; i<end-start; i++) {	
		SDL_RWseek(src, ((start+i)*2352)+16+8, RW_SEEK_SET);
		SDL_RWread(src, &buffer[i*DATA_LENGTH], DATA_LENGTH, 1);
	}

	switch(file_type) {
		case FILE_TIM:
			fileext = "%08x.tim";
			break;
		case FILE_EMD:
			fileext = "%08x.emd";
			break;
	}
	sprintf(filename, fileext, start);

	dst = SDL_RWFromFile(filename, "wb");
	if (!dst) {
		fprintf(stderr, "Can not create %s for writing\n", filename);
		free(buffer);
		return;
	}

	SDL_RWwrite(dst, buffer, length, 1);
	SDL_RWclose(dst);

	free(buffer);
}
