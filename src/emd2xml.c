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
#include <libxml/xmlwriter.h>
#include <SDL.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "file_functions.h"
#include "emd_common.h"
#include "emd1.h"

/*--- Functions prototypes ---*/

int emdToXml(const char *filename);
int getEmdVersion(Uint8 *src, Uint32 srcLen);

int emd1ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc);
void emd1AddSkeleton(Uint8 *src, Uint32 srcLen, xmlNodePtr root);
void emd1AddArmature(xmlNodePtr root, emd1_skel_data_t *emd_skel_data, emd_vertex3_t *emd_skel_relpos, int start_mesh);

int emd2ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc);

int emd3ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc);

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
	xmlDoc *doc;

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

	doc = xmlNewDoc(BAD_CAST "1.0");
	switch(gameVersion) {
		case 1:
			retval = emd1ToXml(srcBuffer, srcBufLen, doc);
			break;
		case 2:
			retval = emd2ToXml(srcBuffer, srcBufLen, doc);
			break;
		case 3:
			retval = emd3ToXml(srcBuffer, srcBufLen, doc);
			break;
		default:
			retval = 1;
			break;
	}

	/* Save if OK */
	if (!retval) {
		xmlSaveFormatFileEnc("re1.xml", doc, "UTF-8", 1);
	}
	xmlFreeDoc(doc);

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

int emd1ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc)
{
	xmlNodePtr root, node;

	printf("Detected RE1 EMD file\n");
	root = xmlNewNode(NULL, BAD_CAST "emd");	
	xmlNewProp(root, BAD_CAST "version", BAD_CAST "1");
	xmlDocSetRootElement(doc, root);

	emd1AddSkeleton(src, srcLen, root);

	/* Animations */
	node = xmlNewNode(NULL, BAD_CAST "animation");
	xmlAddChild(root, node);

	/* Meshes */
	node = xmlNewNode(NULL, BAD_CAST "mesh");
	xmlAddChild(root, node);

	/* TIM image */
	node = xmlNewNode(NULL, BAD_CAST "tim");
	xmlAddChild(root, node);
	xmlNewProp(node, BAD_CAST "filename", BAD_CAST "texture.tim");

	return 0;
}

void emd1AddSkeleton(Uint8 *src, Uint32 srcLen, xmlNodePtr root)
{
	xmlNodePtr node;
	Uint32 *hdr_offsets;
	Uint8 *src_skel;
	emd_skel_header_t *emd_skel_header;
	emd_vertex3_t *emd_skel_relpos;
	emd1_skel_data_t *emd_skel_data;

	hdr_offsets = (Uint32 *) (&src[srcLen-16]);
	src_skel = &src[SDL_SwapLE32(hdr_offsets[EMD1_SKELETON])];

	emd_skel_header = (emd_skel_header_t *) src_skel;
	emd_skel_relpos = (emd_vertex3_t *) (&src_skel[sizeof(emd_skel_header_t)]);
	emd_skel_data = (emd1_skel_data_t *) (&src_skel[SDL_SwapLE16(emd_skel_header->relpos_offset)]);

	node = xmlNewNode(NULL, BAD_CAST "skeleton");
	xmlAddChild(root, node);

	emd1AddArmature(node, emd_skel_data, emd_skel_relpos, 0);
}

void emd1AddArmature(xmlNodePtr root, emd1_skel_data_t *emd_skel_data, emd_vertex3_t *emd_skel_relpos, int start_mesh)
{
	xmlNodePtr node;
	xmlChar buf[32];
	Uint16 num_mesh = SDL_SwapLE16(emd_skel_data[start_mesh].num_mesh);
	Uint16 offset = SDL_SwapLE16(emd_skel_data[start_mesh].offset);
	Uint8 *armature = (Uint8 *) emd_skel_data;
	int i;

	node = xmlNewNode(NULL, BAD_CAST "armature");
	xmlAddChild(root, node);

	xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", start_mesh);
	xmlNewProp(node, BAD_CAST "mesh", buf);

	xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_relpos[start_mesh].x));
	xmlNewProp(node, BAD_CAST "x", buf);
	xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_relpos[start_mesh].y));
	xmlNewProp(node, BAD_CAST "y", buf);
	xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_relpos[start_mesh].z));
	xmlNewProp(node, BAD_CAST "z", buf);

	for (i=0; i<num_mesh; i++) {
		emd1AddArmature(node, emd_skel_data, emd_skel_relpos, armature[offset+i]);
	}
}

/*--- RE2 EMD ---*/

int emd2ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc)
{
	printf("Detected RE2 EMD file\n");
	return 1;
}

/*--- RE3 EMD ---*/

int emd3ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc)
{
	printf("Detected RE3 EMD file\n");
	return 1;
}
