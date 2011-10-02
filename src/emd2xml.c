/*
	EMD to XML

	Copyright (C) 2011	Patrice Mandin

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

#include <libxml/xmlversion.h>
#include <SDL.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "file_functions.h"
#include "emd_common.h"

/*--- Functions prototypes ---*/

int emdToXml(const char *filename);
int getEmdVersion(Uint8 *src, Uint32 srcLen);
int emd1ToXml(Uint8 *src, Uint32 srcLen);
int emd2ToXml(Uint8 *src, Uint32 srcLen);
int emd3ToXml(Uint8 *src, Uint32 srcLen);

/*--- Functions ---*/

int main(int argc, char **argv)
{
	int retval;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.emd\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO)<0) {
		fprintf(stderr, "Can not initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	LIBXML_TEST_VERSION
	xmlInitParser();

	retval = emdToXml(argv[argc-1]);

	xmlCleanupParser();
	xmlMemoryDump();

	SDL_Quit();
	return retval;
}

int emdToXml(const char *filename)
{
	SDL_RWops *src;
	Uint8 *srcBuffer;
	int srcBufLen, gameVersion;
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

	/* Detect which game version */
	gameVersion = getEmdVersion(srcBuffer, srcBufLen);
	switch(gameVersion) {
		case 1:
			retval = emd1ToXml(srcBuffer, srcBufLen);
			break;
		case 2:
			retval = emd2ToXml(srcBuffer, srcBufLen);
			break;
		case 3:
			retval = emd3ToXml(srcBuffer, srcBufLen);
			break;
		default:
			retval = 1;
			break;
	}

	return retval;
}

int getEmdVersion(Uint8 *src, Uint32 srcLen)
{
	emd_header_t *emd_header = (emd_header_t *) src;
	Uint32 hdr_offsets = SDL_SwapLE32(emd_header->offset);
	Uint32 hdr_length = SDL_SwapLE32(emd_header->length);

	/* RE1 does not have emd_header_t
	 * so check if usable as RE2 or RE3 file */
	if ((hdr_offsets + (hdr_length * sizeof(Uint32))) == srcLen) {
		if (hdr_length == 8) {
			/* RE2 */
			return 2;
		}
		/* RE3 */
		return 3;
	}

	/* RE1 */
	return 1;
}

/*--- RE1 EMD ---*/

int emd1ToXml(Uint8 *src, Uint32 srcLen)
{
	printf("Detected RE1 EMD file\n");
	return 1;
}

/*--- RE2 EMD ---*/

int emd2ToXml(Uint8 *src, Uint32 srcLen)
{
	printf("Detected RE2 EMD file\n");
	return 1;
}

/*--- RE3 EMD ---*/

int emd3ToXml(Uint8 *src, Uint32 srcLen)
{
	printf("Detected RE3 EMD file\n");
	return 1;
}
