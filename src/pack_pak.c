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
#include <fcntl.h>

#include <SDL.h>

/*--- Defines ---*/

#define CHUNK_SIZE 32768

#define DECODE_SIZE 35024

#define LZW_STOP	0x100	/* End of stream */
#define LZW_NEXT	0x101	/* Increment bit size */
#define LZW_CLEAR	0x102	/* Clear dictionary */

/*--- Types ---*/

typedef struct {
	long flag;	/* 0: free, 1:in use */
	long index;
	long value;
	int num_bits;
} re1_pack_t;

/*--- Variables ---*/

static Uint8 *dstPointer;
static int dstBufLen;
static int dstOffset;

static re1_pack_t dict[DECODE_SIZE];

static int out_code, out_code_bits;

static Uint8 *curstr;
static int curstr_pos, curstr_len;

/*--- Functions ---*/

static void dict_clear(void)
{
	int i;

	for (i=0; i<DECODE_SIZE; i++) {
		dict[i].flag = 0;
	}
	for (i=0; i<256; i++) {
		dict[i].flag = 1;
		dict[i].value = i;
		dict[i].num_bits = 8;
	}

	out_code = 0x103;
	out_code_bits = 9;
}

static int dict_check(Uint8 new_char)
{
	return -1;
}

static int dict_add(Uint8 new_char)
{
	return -1;
}

static void curstr_clear(void)
{
	curstr_pos = 0;
}

static void curstr_addchar(Uint8 new_char)
{
	if (curstr_pos >= curstr_len-1) {
		int new_len = curstr_len + CHUNK_SIZE;
		curstr = realloc(curstr, new_len);
		if (curstr==NULL) {
			fprintf(stderr, "pak: can not allocate %d bytes\n", new_len);
			return;
		}
		curstr_len = new_len;
	}

	curstr[curstr_pos++] = new_char;
}

static void curstr_set(Uint8 new_char)
{
	if (curstr_pos >= curstr_len-1) {
		int new_len = curstr_len + CHUNK_SIZE;
		curstr = realloc(curstr, new_len);
		if (curstr==NULL) {
			fprintf(stderr, "pak: can not allocate %d bytes\n", new_len);
			return;
		}
		curstr_len = new_len;
	}

	curstr_pos = 1;
	curstr[0] = new_char;
}

static void pak_write_bits(Uint32 value, int num_bits)
{
}

void pak_pack(SDL_RWops *src, Uint8 **dstBufPtr, int *dstLength)
{
	Uint8 src_char;

	*dstBufPtr = dstPointer = NULL;
	*dstLength = dstBufLen = dstOffset = 0;
	curstr_len = 0;

	/* Init base dict */
	dict_clear();

	/* Current string = empty */
	curstr_clear();

	/* While character in source */
	for (;;) {
		if ( SDL_RWread( src, &src_char, 1, 1 ) <= 0) {
			/* Output end of stream */
			pak_write_bits(LZW_STOP, 9);
			break;
		}

		/* if cur_string+src_char in dict */
		if (dict_check(src_char)>=0) {
			/* cur_string += src_char */
			curstr_addchar(src_char);
		} else {
			/* add cur_string+src_char to dict */
			int dict_index = dict_add(src_char);

			/* write cur_string index to output */
			pak_write_bits(dict_index, dict[dict_index].num_bits);

			/* cur_string = src_char */
			curstr_set(src_char);
		}
	}

	/* Free stuff */
	if (curstr) {
		free(curstr);
		curstr=NULL;
	}

	/* Return packed buffer */
	*dstBufPtr = (Uint8 *) dstPointer;
	*dstLength = dstOffset;
}
