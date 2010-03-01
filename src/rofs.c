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
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL.h>

#include "file_functions.h"

/*--- Types ---*/

typedef struct {
	Uint8 unknown[4*5+1];
} rofs_header_t;

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
	Uint8 ident[8];
} rofs_crypt_header_t;

/*--- Const ---*/

const unsigned short base_array[64]={
	0x00e6, 0x01a4, 0x00e6, 0x01c5,
	0x0130, 0x00e8, 0x03db, 0x008b,
	0x0141, 0x018e, 0x03ae, 0x0139,
	0x00f0, 0x027a, 0x02c9, 0x01b0,
	0x01f7, 0x0081, 0x0138, 0x0285,
	0x025a, 0x015b, 0x030f, 0x0335,
	0x02e4, 0x01f6, 0x0143, 0x00d1,
	0x0337, 0x0385, 0x007b, 0x00c6,
	0x0335, 0x0141, 0x0186, 0x02a1,
	0x024d, 0x0342, 0x01fb, 0x03e5,
	0x01b0, 0x006d, 0x0140, 0x00c0,
	0x0386, 0x016b, 0x020b, 0x009a,
	0x0241, 0x00de, 0x015e, 0x035a,
	0x025b, 0x0154, 0x0068, 0x02e8,
	0x0321, 0x0071, 0x01b0, 0x0232,
	0x02d9, 0x0263, 0x0164, 0x0290
};

/*--- Variables ---*/

Uint8 rofs_header[4096];

Uint8 tmp4k[4096+256];

/*--- Function prototypes ---*/

void create_dirs(const char *level1, const char *level2);

void list_files(const char *filename);
void extract_file(SDL_RWops *src, const char *filename, rofs_file_header_t *file_hdr);

Uint8 re3_next_key(Uint32 *key);
void decrypt_block(Uint8 *src, Uint32 key, Uint32 length);

Uint32 depack_block(Uint8 *src, Uint32 length);

/*--- Functions ---*/

int main(int argc, char **argv)
{
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
	return 0;
}

void create_dirs(const char *level1, const char *level2)
{
	char filename[512];

	mkdir(level1, 0755);

	sprintf(filename, "%s/%s", level1, level2);
	mkdir(filename, 0755);
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

	create_dirs(dir_level1_name, dir_level2_name);

	offset = SDL_SwapLE32(dir_level2.offset)*8;
	SDL_RWseek(src, offset, RW_SEEK_SET);

	/* Number of files */
	SDL_RWread(src, &num_files, 4, 1);
	num_files = SDL_SwapLE32(num_files);
	offset = SDL_RWtell(src);

	/*printf("files: %d\n", num_files);*/

	/*printf("Offset\t\tLength\t\tName\n");*/
	for (i=0; i<num_files; i++) {
		rofs_file_header_t file_hdr;
		char filename[512];
		int j;

		SDL_RWseek(src, offset, RW_SEEK_SET);

		/* Read file header */
		SDL_RWread(src, &file_hdr, sizeof(file_hdr), 1);
		file_hdr.length = SDL_SwapLE32(file_hdr.length);
		file_hdr.offset = SDL_SwapLE32(file_hdr.offset) * 8;

		/* Read file name */
		sprintf(filename, "%s/%s/", dir_level1_name, dir_level2_name);
		j = strlen(filename);
		for (;;) {
			SDL_RWread(src, &filename[j], 1,1);
			if (filename[j] == 0) {
				break;
			}
			j++;
		}

		offset = SDL_RWtell(src);

		/*printf(" file %d: %s\n", i, filename);*/
		/*printf("0x%08x\t0x%08x\t%s\n",
			file_hdr.offset, file_hdr.length, filename);*/

		extract_file(src, filename, &file_hdr);
	}

	SDL_RWclose(src);
}

void extract_file(SDL_RWops *src, const char *filename, rofs_file_header_t *file_hdr)
{
	rofs_crypt_header_t crypt_hdr;
	int i, compressed;
	Uint32 *array_keys, *array_length;
	Uint32 offset, dstBufLen;
	Uint8 *dstBuffer;

	SDL_RWseek(src, file_hdr->offset, RW_SEEK_SET);
	offset = SDL_RWtell(src);
	SDL_RWread(src, &crypt_hdr, sizeof(rofs_crypt_header_t), 1);

	for (i=0; i<8; i++) {
		crypt_hdr.ident[i] ^= crypt_hdr.ident[7];
	}
	compressed = (strcmp("Hi_Comp", crypt_hdr.ident)==0);

	/* Read decryption keys */
	array_keys = calloc(SDL_SwapLE16(crypt_hdr.num_keys)*2, sizeof(Uint32));
	if (!array_keys) {
		fprintf(stderr, "Can not allocate memory for keys\n");
		return;
	}
	array_length = &array_keys[SDL_SwapLE16(crypt_hdr.num_keys)];
	SDL_RWread(src, array_keys, SDL_SwapLE16(crypt_hdr.num_keys)*2, sizeof(Uint32));
	for (i=0; i<SDL_SwapLE16(crypt_hdr.num_keys)*2; i++) {
		array_keys[i] = SDL_SwapLE32(array_keys[i]);
	}

	/* Go to start of file */
	offset += SDL_SwapLE16(crypt_hdr.offset);
	SDL_RWseek(src, offset, RW_SEEK_SET);

	dstBufLen = SDL_SwapLE32(crypt_hdr.length);
	dstBuffer = malloc(dstBufLen+16);
	if (!dstBuffer) {
		fprintf(stderr, "Can not allocate memory for file\n");
		free(array_keys);
		return;
	}

	printf("Extracting %s, length %d...\n", filename, dstBufLen);

	offset = 0;
	for (i=0; i<SDL_SwapLE16(crypt_hdr.num_keys); i++) {
		Uint32 block_length = array_length[i];

		if (offset+block_length>dstBufLen) {
			block_length = dstBufLen-offset;
		}

		/*printf(" Reading at offset %d, len %d (finish %d)\n", offset,block_length,offset+block_length);*/
		SDL_RWread(src, &dstBuffer[offset], block_length, 1);

		/* Decrypt */
		decrypt_block(&dstBuffer[offset], array_keys[i], block_length);

		/* Depack */
		if (compressed) {
			block_length = depack_block(&dstBuffer[offset], block_length);
		}

		offset += block_length;
	}

	save_file(filename, dstBuffer, dstBufLen);
 
	free(dstBuffer);
	free(array_keys);
}

Uint8 re3_next_key(Uint32 *key)
{
	*key *= 0x5d588b65;
	*key += 0x8000000b;

	return (*key >> 24);
}

void decrypt_block(Uint8 *src, Uint32 key, Uint32 length)
{
	Uint8 xor_key, base_index;
	int i, block_index;

	xor_key = re3_next_key(&key);
	base_index = re3_next_key(&key) % 0x3f;

	block_index = 0;
	for (i=0; i<length; i++) {
		if (block_index>base_array[base_index]) {
			base_index = re3_next_key(&key) % 0x3f;
			xor_key = re3_next_key(&key);
			block_index = 0;
		}
		src[i] ^= xor_key;
		block_index++;
	}
}

Uint32 depack_block(Uint8 *dst, Uint32 length)
{
	int srcNumBit, srcIndex, tmpIndex, dstIndex;
	int i, value, value2, tmpStart, tmpLength;
	Uint8 *src;

	for (i=0; i<256; i++) {
 		memset(&tmp4k[i*16], i, 16); 
	}
	memset(&tmp4k[4096], 0, 256);

	/* Copy source to a temp copy */
	src = (Uint8 *) malloc(length);
	if (!src) {
		fprintf(stderr, "Can not allocate memory for depacking\n");
		return length;
	}
	memcpy(src, dst, length);

	/*printf("Depacking %08x to %08x, len %d\n", src,dst,length);*/

	srcNumBit = 0;
	srcIndex = 0;
	tmpIndex = 0;
	dstIndex = 0;
	while (srcIndex<length) {
		srcNumBit++;

		value = src[srcIndex++] << srcNumBit;
		if (srcIndex<length) {
			value |= src[srcIndex] >> (8-srcNumBit);
		}

		if (srcNumBit==8) {
			srcIndex++;
			srcNumBit = 0;
		}

		if ((value & (1<<8))==0) {
			dst[dstIndex++] = tmp4k[tmpIndex++] = value;
		} else {
			value2 = (src[srcIndex++] << srcNumBit) & 0xff;
			value2 |= src[srcIndex] >> (8-srcNumBit);

			tmpLength = (value2 & 0x0f)+2;

			tmpStart = (value2 >> 4) & 0xfff;
			tmpStart |= (value & 0xff) << 4;

			memcpy(&dst[dstIndex], &tmp4k[tmpStart], tmpLength);
			memcpy(&tmp4k[tmpIndex], &dst[dstIndex], tmpLength);

			dstIndex += tmpLength;
			tmpIndex += tmpLength;
		}

		if (tmpIndex>=4096) {
			tmpIndex = 0;
		}
	}

	/*printf("Depacked to %d len\n", dstIndex-1);*/

	free(src);
	return dstIndex-1;
}
