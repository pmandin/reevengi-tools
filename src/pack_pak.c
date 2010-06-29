/*
	PAK file packer
	This is a variant of LZW

	Copyright (C) 2010	Patrice Mandin
	Copyright (C) 1989	Mark R. Nelson

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
#define LZW_FIRST	0x103	/* First free code for string */

/*--- Types ---*/

typedef struct {
	int len;	/* Length of encoded string */
	Uint8 *enc_str;	/* Encoded string */
} re1_pack_t;

/*--- Variables ---*/

static Uint8 *dstPointer;
static int dstBufLen;
static int dstOffset;
static int dstBit;

static re1_pack_t dict[DECODE_SIZE];

static int out_code, out_code_bits;

static Uint8 *curstr;
static int curstr_pos, curstr_len;

/*--- Functions prototypes ---*/

static void dict_clear(void);
static void dict_genstr(Uint8 new_char);
static void dict_check(int *stronly, int *strandchar);

static void curstr_clear(void);
static void curstr_addchar(Uint8 new_char);
static void curstr_set(Uint8 new_char);

static void pak_write_bits(Uint32 value, int num_bits);

/*--- Functions ---*/

static void dict_clear(void)
{
	int i;

	for (i=LZW_FIRST; i<DECODE_SIZE; i++) {
		if (dict[i].enc_str) {
			free(dict[i].enc_str);
			dict[i].enc_str = NULL;
			dict[i].len = 0;
		}
	}

	out_code = LZW_FIRST;
	out_code_bits = 9;
}

static void dict_genstr(Uint8 new_char)
{
	if (dict[out_code].len <= curstr_pos ) {
		dict[out_code].len = curstr_pos + 1;
		dict[out_code].enc_str = realloc(dict[out_code].enc_str, dict[out_code].len);
	}

	if (curstr_pos>0) {
		memcpy(dict[out_code].enc_str, curstr, curstr_pos);
	}
	dict[out_code].enc_str[curstr_pos] = new_char;
}

static void dict_check(int *stronly, int *strandchar)
{
	int i;

	*stronly = *strandchar = -1;

	/* Check single char first */
	if (dict[out_code].len == 1) {
		*strandchar = dict[out_code].enc_str[0];
	}
	if (curstr_pos==1) {
		*stronly = curstr[0];
	}

	/* Check each prefix first */
	for (i=LZW_FIRST; i<out_code; i++) {
		if (dict[i].len == curstr_pos) {
			if (memcmp(dict[i].enc_str, curstr, curstr_pos) == 0) {
				*stronly = i;
			}
		}
		if (dict[i].len == dict[out_code].len) {
			if (memcmp(dict[i].enc_str, dict[out_code].enc_str, dict[out_code].len) == 0) {
				*strandchar = i;
			}
		}
	}
}

static void curstr_clear(void)
{
	curstr_pos = 0;
}

static void curstr_addchar(Uint8 new_char)
{
	if (curstr_pos >= curstr_len-1) {
		curstr_len += CHUNK_SIZE;
		curstr = realloc(curstr, curstr_len);
		if (curstr==NULL) {
			fprintf(stderr, "pak: can not allocate %d bytes\n", curstr_len);
			return;
		}
	}

	curstr[curstr_pos++] = new_char;
}

static void curstr_set(Uint8 new_char)
{
	curstr_pos = 0;
	curstr_addchar(new_char);
}

static int output_bit_count = 0;
static Uint32 output_bit_buffer = 0;

static void pak_write_bits(Uint32 value, int num_bits)
{
	output_bit_buffer |= (Uint32) value << (32-num_bits-output_bit_count);
	output_bit_count += num_bits;

	while (output_bit_count>=8) {
		if ((dstPointer==NULL) || (dstOffset>=dstBufLen)) {
			dstBufLen += CHUNK_SIZE;
			dstPointer = realloc(dstPointer, dstBufLen);
			if (dstPointer==NULL) {
				fprintf(stderr, "pak: can not allocate %d bytes\n", dstBufLen);
				return;
			}
		}

		dstPointer[dstOffset++] = output_bit_buffer >> 24;
		output_bit_buffer <<= 8;
		output_bit_count -= 8;
	}
}

static int
is_pot(unsigned x)
{
   return (x & (x - 1)) == 0;
}

void pak_pack(SDL_RWops *src, Uint8 **dstBufPtr, int *dstLength)
{
	Uint8 src_char;
	int i, dict_str, dict_strandchar;
	Uint32 srclen, srcOffset = 0;

	*dstBufPtr = dstPointer = NULL;
	*dstLength = dstBufLen = dstOffset = 0;
	dstBit = 7;
	curstr_len = 0;

	SDL_RWseek(src, 0, RW_SEEK_END);
	srclen = SDL_RWtell(src);
	SDL_RWseek(src, 0, RW_SEEK_SET);

	/* Init base dict */
	memset(dict, 0, sizeof(dict));
	dict_clear();

	/* Current string = empty */
	curstr_clear();

	/* While character in source */
	for (;;) {
		if ( SDL_RWread( src, &src_char, 1, 1 ) <= 0) {
			break;
		}
		++srcOffset;

		/* Generate new string = cur_string+src_char */
		dict_genstr(src_char);

		/* if cur_string+src_char in dict */
		dict_check(&dict_str, &dict_strandchar);

		if (dict_strandchar>=0) {
			/* cur_string += src_char */
			curstr_addchar(src_char);
		} else {
			/* Need more bits ? */
			if (is_pot(out_code)) {
				pak_write_bits(LZW_NEXT, out_code_bits);
				++out_code_bits;
			}

			/* write cur_string index to output */
			pak_write_bits(dict_str, out_code_bits);

			/* add cur_string+src_char to dict */
			++out_code;

			/* cur_string = src_char */
			curstr_set(src_char);
		}

		if ((srcOffset & 31)==0) {
			printf("%d %%\r", (srcOffset*100)/srclen);
		}
	}
	printf("\n");

	/* Output last code */
	pak_write_bits(dict_strandchar, out_code_bits);
	/* Output end of stream */
	pak_write_bits(LZW_STOP, out_code_bits);
	/* Flush remaining bits */
	pak_write_bits(0, output_bit_count+8);

	/* Free stuff */
	if (curstr) {
		free(curstr);
	}

	for (i=LZW_FIRST; i<DECODE_SIZE; i++) {
		if (dict[i].enc_str) {
			free(dict[i].enc_str);
		}
	}

	/* Return packed buffer */
	*dstBufPtr = (Uint8 *) dstPointer;
	*dstLength = dstOffset;
}
