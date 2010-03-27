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

#include "md5.h"
#include "background_tim.h"

/*--- Defines ---*/

#define EXTRACT_FILES	1
/*#define EXTRACT_FOR_SOURCE 1*/

#define FILE_TIM	0
#define FILE_EMD	1

#define DATA_LENGTH 2048
#define MAX_FILE_SIZE (512<<10)

/*--- Types ---*/

typedef struct {
	const char *value;	/* MD5 value */
	const char *filename;	/* Filename */
} md5_check_t;

/*--- Constants ---*/

const md5_check_t md5_checks[]={
	{"9bcc78189acac849f6cda0986ce084d9","ROOM/EMD/EM10.EMD"},
	{"02613fdd2a9402284c7c906283d80973","ROOM/EMD/EM10.TIM"},
	{"b3172c3aa228533c637e328d84a4a0e7","ROOM/EMD/EM11.EMD"},
	{"0c29e033124028068ca56da61a19c4a7","ROOM/EMD/EM11.TIM"},
	{"c16bbc8652accffca210b3047dcaf810","ROOM/EMD/EM12.EMD"},
	{"7e04298f5a5adecb14b9d60c9b2d163e","ROOM/EMD/EM12.TIM"},
	{"523090faa3f1e3bc3842fda8c13a20fb","ROOM/EMD/EM13.EMD"},
	{"db00767cce785ef6da1c8e5755fbfe2f","ROOM/EMD/EM13.TIM"},
	{"6e8153cd2db6405aeb2671471b22bb35","ROOM/EMD/EM14.EMD"},
	{"1225b45f80f7a991830d39ec23fd3e20","ROOM/EMD/EM14.TIM"},
	{"318b4f532fd391a484ea28efaefa1c70","ROOM/EMD/EM15.EMD"},
	{"002bf41b7c8dab37cc177aba973b910f","ROOM/EMD/EM15.TIM"},
	{"3401c8a6295f9a965095377bef78c093","ROOM/EMD/EM16.EMD"},
	{"098a614f991bdaefdb706fac38900a89","ROOM/EMD/EM16.TIM"},
	{"4b46fb238b5078434093950b7d3539aa","ROOM/EMD/EM17.EMD"},
	{"a4acbb1a32c3301b9a722cf7f4d50f4a","ROOM/EMD/EM17.TIM"},
	{"ae9b192d93a9af6b9270cdd279d388c1","ROOM/EMD/EM18.EMD"},
	{"bd64827a53ac83376a37fb8540e42239","ROOM/EMD/EM18.TIM"},
	{"f62b85aa2410aba623052652978e9a4b","ROOM/EMD/EM19.EMD"},
	{"d04ef240fd3157785bd3f46feabc0de6","ROOM/EMD/EM19.TIM"},
	{"f8a39d03ed7f568b78cf5232a351f1d4","ROOM/EMD/EM1A.EMD"},
	{"47f3d7b46cc1e57d637ac328e17d7a26","ROOM/EMD/EM1A.TIM"},
	{"fa7e9e13ae269c1507c5abae56368f18","ROOM/EMD/EM1B.EMD"},
	{"d0188eb8a5e34bdd0f78a899e1b0bcc5","ROOM/EMD/EM1B.TIM"},
	{"f3bb3369dc66a3b3f44643a2fff9dec6","ROOM/EMD/EM1C.EMD"},
	{"8f37dfd58aab16ea2bb67d68529d5a21","ROOM/EMD/EM1C.TIM"},
	{"b01ebf1df508fb1d56910559c7251a11","ROOM/EMD/EM1D.EMD"},
	{"907c31e1adbe9807c256a4842cfb9a15","ROOM/EMD/EM1D.TIM"},
	{"3401c8a6295f9a965095377bef78c093","ROOM/EMD/EM1E.EMD"},
	{"098a614f991bdaefdb706fac38900a89","ROOM/EMD/EM1E.TIM"},
	{"c94f3ceb8c613d4f9ef4e3838fd68b26","ROOM/EMD/EM1F.EMD"},
	{"fcb5c49f6ebba4330c0376ed65b751f9","ROOM/EMD/EM1F.TIM"},
	{"0e1ec3e2b13de764b9b9cc9850eeb7e1","ROOM/EMD/EM20.EMD"},
	{"b8ad398255ae64ebe1eebb77ce0984a9","ROOM/EMD/EM20.TIM"},
	{"84446e90460e558418d500902f99bd0a","ROOM/EMD/EM21.EMD"},
	{"413c9653df474b05f18f778a6cf5a1a2","ROOM/EMD/EM21.TIM"},
	{"ad896f4aaefa49e3186ef388f3affdd8","ROOM/EMD/EM22.EMD"},
	{"aeaf4bac057e0b19b23e85a707bf4631","ROOM/EMD/EM22.TIM"},
	{"05c86b22e071ac58190219bba7af072c","ROOM/EMD/EM23.EMD"},
	{"77d6dc3597fedd07dee5c2d4fddc3ab6","ROOM/EMD/EM23.TIM"},
	{"6f168094b6c9838546398d9c21d0a055","ROOM/EMD/EM24.EMD"},
	{"6a5aae2906ea61bd978513811e7dc050","ROOM/EMD/EM24.TIM"},
	{"97d0268b25e62a84d1dfe51498e04a4d","ROOM/EMD/EM25.EMD"},
	{"33d63344a232dc5292c5e9d3188a293b","ROOM/EMD/EM25.TIM"},
	{"84dde72548a5b980242bd268271d0f3c","ROOM/EMD/EM26.EMD"},
	{"09a18a39781045f65c527c13ce6dbf3b","ROOM/EMD/EM26.TIM"},
	{"9311c8e236bc04939c2e4607a5b105b6","ROOM/EMD/EM27.EMD"},
	{"7514ff8bbcbaca75b2b87c2d3c61c343","ROOM/EMD/EM27.TIM"},
	{"6bb3c76fa7e079bfa59095bfedde0dc7","ROOM/EMD/EM28.EMD"},
	{"1dde25c43a8027bae6e3c221b88c909f","ROOM/EMD/EM28.TIM"},
	{"942cd7da477c36402c1d95f8badefa28","ROOM/EMD/EM2C.EMD"},
	{"9fd98d162d710fd39e1a2d19ef84b25f","ROOM/EMD/EM2C.TIM"},
	{"9aa73cd765be331e3b33e5325b767988","ROOM/EMD/EM2D.EMD"},
	{"c965e1faa886bde5f873a4d92da0b35b","ROOM/EMD/EM2D.TIM"},
	{"f1bb239b039b546ff02fdb656a133f8a","ROOM/EMD/EM2E.EMD"},
	{"4bbe791db00d84a618abce4d23d66d99","ROOM/EMD/EM2E.TIM"},
	{"ee2c79a51839e69a9677640f4d179f60","ROOM/EMD/EM2F.EMD"},
	{"b414d04bdb0fd4c4fb20927a1c85f4b8","ROOM/EMD/EM2F.TIM"},
	{"b3f81d8b8e638bb6bf7612f10b33d868","ROOM/EMD/EM30.EMD"},
	{"4cf5acb565dc57c9456f1961f6c3e284","ROOM/EMD/EM30.TIM"},
	{"ef3a11eae0d61dbfa7e2e620b698b0ca","ROOM/EMD/EM32.EMD"},
	{"a15eb28823b1463954861ba1100ca2b7","ROOM/EMD/EM32.TIM"},
	{"d26111f8fc18044000f655da29a5746b","ROOM/EMD/EM33.EMD"},
	{"a3c4eccccca8de15f006e7f6d13a6b7e","ROOM/EMD/EM33.TIM"},
	{"d820f094e6d85cc822dc1af3de626967","ROOM/EMD/EM34.EMD"},
	{"40c92c76cf55988fc8aaf6903fb74d6e","ROOM/EMD/EM34.TIM"},
	{"9f673280ee9bd2647eaec6c3512ba53b","ROOM/EMD/EM35.EMD"},
	{"fef483ea0d083f025528f572ced803b8","ROOM/EMD/EM35.TIM"},
	{"be080f67d85e8af8de3968f8cdccb502","ROOM/EMD/EM36.EMD"},
	{"f381d1228e1f293b4aba8815c77d0319","ROOM/EMD/EM36.TIM"},
	{"cb2caf355f68ba54faac06d8824db816","ROOM/EMD/EM37.EMD"},
	{"0166fe0b59d0ccd56ff6370a20d5cbe2","ROOM/EMD/EM37.TIM"},
	{"2d59fb9e12fc0bfd5f939e94dc4a215e","ROOM/EMD/EM38.EMD"},
	{"3d97aa2a862a825682faf8326185d151","ROOM/EMD/EM38.TIM"},
	{"13a0166a6bf6f57fdf5c0ae106aa7ca8","ROOM/EMD/EM39.EMD"},
	{"35203d7fc448e4a7b5398a9c52f9c840","ROOM/EMD/EM39.TIM"},
	{"1c6e07531be585a2cf09c2e27bca859c","ROOM/EMD/EM3A.EMD"},
	{"9043a2e9d4921f44d172adc219a2e1d7","ROOM/EMD/EM3A.TIM"},
	{"3bc3dac621d67b678141a3f606c303e2","ROOM/EMD/EM3B.EMD"},
	{"0166fe0b59d0ccd56ff6370a20d5cbe2","ROOM/EMD/EM3B.TIM"},
	{"85499323e9c8bf4887aeb1ba8bd49a50","ROOM/EMD/EM3E.EMD"},
	{"4a238804882f4fcacb9b20f13d117540","ROOM/EMD/EM3E.TIM"},
	{"f5efe027786879d0f4f24a438fa21edc","ROOM/EMD/EM3F.EMD"},
	{"4c32ccd33e33ea4bd640d6740b98f975","ROOM/EMD/EM3F.TIM"},
	{"7c3b495f2b1f9e1f4c23bc97b4e43f74","ROOM/EMD/EM40.EMD"},
	{"4edf797ab6cf75f49806499282b32ced","ROOM/EMD/EM40.TIM"},
	{"fb3b79cd7fc65baf6327c3574b8c551b","ROOM/EMD/EM50.EMD"},
	{"8a6b23c802dda1934922d00d5acfe3e8","ROOM/EMD/EM50.TIM"},
	{"f8dc87952885e36a41663189f3feaf56","ROOM/EMD/EM51.EMD"},
	{"bf68961bdd817cf8e827631d23127cef","ROOM/EMD/EM51.TIM"},
	{"10482a5caf2090c12d0481819bad5c4f","ROOM/EMD/EM52.EMD"},
	{"8ae1eb4e8c9a4b71dba96899da284397","ROOM/EMD/EM52.TIM"},
	{"e666720fd6d71c7acb9ab94bf12e43e0","ROOM/EMD/EM53.EMD"},
	{"58f6405de070393b1a1978fb2add9832","ROOM/EMD/EM53.TIM"},
	{"b756093ae4ecf24371ae578402cc4bd2","ROOM/EMD/EM54.EMD"},
	{"9cbfad29fbdb8b9c9c455840eab2d8bb","ROOM/EMD/EM54.TIM"},
	{"5ecff12c4ec3f4a3f7f5722c66b9755b","ROOM/EMD/EM55.EMD"},
	{"4bbe791db00d84a618abce4d23d66d99","ROOM/EMD/EM55.TIM"},
	{"152145fbf233f55128af760a289cf5b0","ROOM/EMD/EM56.EMD"},
	{"983c220257e26c614616d4892809eb79","ROOM/EMD/EM56.TIM"},
	{"fa75db12f62d6f89af75216e45c11758","ROOM/EMD/EM57.EMD"},
	{"b414d04bdb0fd4c4fb20927a1c85f4b8","ROOM/EMD/EM57.TIM"},
	{"95b9255cd0d01b927a750424a7580f3c","ROOM/EMD/EM58.EMD"},
	{"6589fe16175c3695c127baf1a5f7f571","ROOM/EMD/EM58.TIM"},
	{"a6ac51db0209ace9d332d9c22157104d","ROOM/EMD/EM59.EMD"},
	{"696356ddcf30d8245c915f6deada485d","ROOM/EMD/EM59.TIM"},
	{"c05d9862d6d259d36a2cfdf9b8c634cd","ROOM/EMD/EM5A.EMD"},
	{"db10b5519defebb6d388fa219d0985f3","ROOM/EMD/EM5A.TIM"},
	{"c07ca5c8773e7ef30a6b197d6f977729","ROOM/EMD/EM5B.EMD"},
	{"0ce4cd9daa73d3afb71e107aba1520d0","ROOM/EMD/EM5B.TIM"},
	{"80e51a1b7278e1578a4e1234c44e91df","ROOM/EMD/EM5C.EMD"},
	{"3d3e73855a963156f24b5f9049dd7d23","ROOM/EMD/EM5C.TIM"},
	{"f74bdc7cb7c1c08cd2012024affa321f","ROOM/EMD/EM5D.EMD"},
	{"dc777650bacc4d6855f223222511d3f2","ROOM/EMD/EM5D.TIM"},
	{"6a312fd033ebeaa92605f35f2cdebf39","ROOM/EMD/EM5E.EMD"},
	{"e02b686deeb531f82990e2105f7c9b75","ROOM/EMD/EM5E.TIM"},
	{"5584d19d51d1dd6321929743f9371c11","ROOM/EMD/EM5F.EMD"},
	{"5c7dbae23df4467d97c8ff2f3097bdb5","ROOM/EMD/EM5F.TIM"},
	{"103a8891cdae8eac38a7c33bda9ebbc0","ROOM/EMD/EM60.EMD"},
	{"e7b52f10d424975099864ab88fd6d963","ROOM/EMD/EM60.TIM"},
	{"83811e5554f05d45743a908becfbb5cc","ROOM/EMD/EM61.EMD"},
	{"b1e85ed30a337ef650619018994aa2b0","ROOM/EMD/EM61.TIM"},
	{"fbd62743ad76cddec47b783507809e55","ROOM/EMD/EM62.EMD"},
	{"4f7972b7572fb9b2946c6a8fee3d1626","ROOM/EMD/EM62.TIM"},
	{"b6a14ccaba32f6eea0a26d67d8ee3865","ROOM/EMD/EM63.EMD"},
	{"e349e9d3650ebd34b30f0a5061c13fd3","ROOM/EMD/EM63.TIM"},
	{"3fe9ce5b0c51a5f0d73a67bb6ae8b3bf","ROOM/EMD/EM64.EMD"},
	{"e129c22479b5c8b9c839f39a2e959afe","ROOM/EMD/EM64.TIM"},
	{"018fe203e7861de009795ef04271c85e","ROOM/EMD/EM65.EMD"},
	{"ee7ffde6488972d391ae862ad7d9a49a","ROOM/EMD/EM65.TIM"},
	{"de0a2d754501781550af8193d9d2ba2a","ROOM/EMD/EM66.EMD"},
	{"f8ce609e6c4f7f8f9ad79e4ad9066ac8","ROOM/EMD/EM66.TIM"},
	{"8ec7f6fb2ba954aa9b155bd10a814415","ROOM/EMD/EM67.EMD"},
	{"c540b4355f0463220407c9b28bf76a05","ROOM/EMD/EM67.TIM"},
	{"c07ca5c8773e7ef30a6b197d6f977729","ROOM/EMD/EM70.EMD"},
	{"20bf8d887b0ab13de60ad034412d55f0","ROOM/EMD/EM70.TIM"},
	{"fbd62743ad76cddec47b783507809e55","ROOM/EMD/EM71.EMD"},
	{"53d47acf108f767564929a2507cc5616","ROOM/EMD/EM71.TIM"}
};

/*--- Functions prototypes ---*/

int browse_iso(const char *filename);
int get_sector_size(SDL_RWops *src);
void extract_file(SDL_RWops *src, Uint32 start, Uint32 end, int block_size, int file_type);

Uint32 get_tim_length(Uint8 *buffer, Uint32 buflen);
Uint32 get_emd_length(Uint8 *buffer, Uint32 buflen);

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
	int block_size, stop_extract=0;
	int i, extract_flag = 0, file_type = FILE_TIM, new_file_type = FILE_TIM;
	Uint32 start=0,end=0;

	src = SDL_RWFromFile(filename, "rb");
	if (!src) {
		fprintf(stderr, "Can not open %s for reading\n", filename);
		return 1;
	}

	block_size = get_sector_size(src);
	printf("Sector size: %d\n", block_size);

	start = end = 0;
	offset = (block_size == 2352) ? 16+8 : 8;
	for (i=0; !stop_extract; offset+=block_size, i++) {
		Uint32 value;

		SDL_RWseek(src, offset, RW_SEEK_SET);
		if (SDL_RWread(src, data, block_size, 1) != 1) {
			stop_extract=1;
			continue;
		}

		value = (data[3]<<24)|
			(data[2]<<16)|
			(data[1]<<8)|
			data[0];

		if (value == MAGIC_TIM) {
			/* TIM image ? */
			value = (data[4+3]<<24)|
				(data[4+2]<<16)|
				(data[4+1]<<8)|
				data[4];

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
			value = (data[4+3]<<24)|
				(data[4+2]<<16)|
				(data[4+1]<<8)|
				data[4];

			if (value==0x0f) {
				printf("Sector %d (offset 0x%08x): EMD model (maybe)\n",i,offset);
				end = i;
				extract_flag = 1;
				new_file_type = FILE_EMD;
			}
		} else if ((i-start)*block_size>=MAX_FILE_SIZE) {
			end = i;
			extract_flag = 1;
			new_file_type = -1;
		}

		if ((start!=0) && (end!=0) && extract_flag) {
			if (file_type != -1) {
#ifdef EXTRACT_FILES
				extract_file(src, start,end,block_size, file_type);
#endif
#ifdef EXTRACT_FOR_SOURCE
				fprintf(stderr,"\t{%d,%d,\"\"},\n",start,end-start);
#endif
			}
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

int get_sector_size(SDL_RWops *src)
{
	char tmp[12];
	const char xamode[12]={0,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0};

	SDL_RWseek(src, 0, RW_SEEK_SET);
	SDL_RWread(src, tmp, 12, 1);
	if (memcmp(tmp, xamode, 12) != 0) {
		return 2336;
	}

	SDL_RWseek(src, 2352, RW_SEEK_SET);
	SDL_RWread(src, tmp, 12, 1);
	if (memcmp(tmp, xamode, 12) != 0) {
		return 2336;
	}

	return 2352;
}

void extract_file(SDL_RWops *src, Uint32 start, Uint32 end, int block_size, int file_type)
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
		if (block_size==2352) {
			SDL_RWseek(src, ((start+i)*2352)+16+8, RW_SEEK_SET);
		} else {
			SDL_RWseek(src, ((start+i)*2336)+8, RW_SEEK_SET);
		}
		SDL_RWread(src, &buffer[i*DATA_LENGTH], DATA_LENGTH, 1);
	}

	switch(file_type) {
		case FILE_TIM:
			fileext = "%08x.tim";
			length = get_tim_length(buffer, length);
			break;
		case FILE_EMD:
			fileext = "%08x.emd";
			length = get_emd_length(buffer, length);
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

Uint32 get_tim_length(Uint8 *buffer, Uint32 buflen)
{
	tim_header_t *tim_header = (tim_header_t *) buffer;
	tim_size_t *tim_size;
	int w,h;
	int num_palettes, num_colors, img_offset;

	img_offset = SDL_SwapLE32(tim_header->offset) + 20;

	tim_size = (tim_size_t *) (&((Uint8 *) buffer)[img_offset-4]);
	w = SDL_SwapLE16(tim_size->width);
	h = SDL_SwapLE16(tim_size->height);

	return img_offset+(w*h*2);
}

Uint32 get_emd_length(Uint8 *buffer, Uint32 buflen)
{
	return buflen;
}
