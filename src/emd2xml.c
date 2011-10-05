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
#include "emd2.h"

/*--- Functions prototypes ---*/

int emdToXml(const char *filename);
int getEmdVersion(Uint8 *src, Uint32 srcLen);

int emd1ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc, const char *filename);
void emd1AddSkeleton(Uint8 *src, Uint32 srcLen, xmlNodePtr root);
void emd1AddArmature(xmlNodePtr root, emd_armature_header_t *emd_skel_data, emd_vertex3_t *emd_skel_relpos, int start_mesh);
Uint32 emd1GetNumMovements(Uint8 *src, Uint32 srcLen);
void emd1AddAnimation(Uint8 *src, Uint32 srcLen, xmlNodePtr root);
void emd1AddModel(Uint8 *src, Uint32 srcLen, xmlNodePtr root);
void emd1AddTim(Uint8 *src, Uint32 srcLen, xmlNodePtr root, const char *filename);
void emd1AddModelVertices(emd_vertex4_t *vtx, Uint32 count, xmlNodePtr root);
void emd1AddModelNormals(emd_vertex4_t *vtx, Uint32 count, xmlNodePtr root);

int emd2ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc);
void emd2AddSkeleton(Uint8 *src, Uint32 srcLen, int num_skel, xmlNodePtr root);
void emd2AddAnimation(Uint8 *src, Uint32 srcLen, int num_anim, xmlNodePtr root);
Sint16 emd2Read12Bits(Uint8 *array, int index);
void emd2AddModel(Uint8 *src, Uint32 srcLen, xmlNodePtr root);
void emd2AddModelTriangles(Uint8 *src, emd2_model_triangle_t *model_tri, xmlNodePtr root);
void emd2AddModelQuads(Uint8 *src, emd2_model_quad_t *model_quad, xmlNodePtr root);
void emd2AddModelTri(emd2_triangle_t *tri, emd2_triangle_tex_t *tri_tex, Uint32 count, xmlNodePtr root);
void emd2AddModelQuad(emd2_quad_t *quad, emd2_quad_tex_t *quad_tex, Uint32 count, xmlNodePtr root);

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
	char *dst_filename;

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
			retval = emd1ToXml(srcBuffer, srcBufLen, doc, filename);
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
		dst_filename = get_filename_ext(filename, ".xml");
		if (dst_filename) {
			xmlSaveFormatFileEnc(dst_filename, doc, "UTF-8", 1);
		}
		free(dst_filename);
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

int emd1ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc, const char *filename)
{
	xmlNodePtr root, node;

	printf("Detected RE1 EMD file\n");
	root = xmlNewNode(NULL, BAD_CAST "emd");	
	xmlNewProp(root, BAD_CAST "version", BAD_CAST "1");
	xmlDocSetRootElement(doc, root);

	node = xmlNewNode(NULL, BAD_CAST "skeleton");
	xmlAddChild(root, node);
	emd1AddSkeleton(src, srcLen, node);

	/* Animations */
	node = xmlNewNode(NULL, BAD_CAST "animation");
	xmlAddChild(root, node);
	emd1AddAnimation(src, srcLen, node);

	/* Meshes */
	node = xmlNewNode(NULL, BAD_CAST "model");
	xmlAddChild(root, node);
	emd1AddModel(src, srcLen, node);

	/* TIM image */
	node = xmlNewNode(NULL, BAD_CAST "tim");
	xmlAddChild(root, node);
	emd1AddTim(src, srcLen, node, filename);

	return 0;
}

void emd1AddSkeleton(Uint8 *src, Uint32 srcLen, xmlNodePtr root)
{
	emd1_directory_t *emd1_dir;
	Uint8 *src_skel, *src_move;
	emd_skel_header_t *emd_skel_header;
	emd_vertex3_t *emd_skel_relpos;
	emd_armature_header_t *emd_skel_data;
	int i, j, move_size;
	xmlNodePtr node;
	xmlChar buf[32];

	emd1_dir = (emd1_directory_t *) (&src[srcLen-sizeof(emd1_directory_t)]);

	/*--- Skeleton ---*/
	src_skel = &src[SDL_SwapLE32(emd1_dir->skeleton)];

	emd_skel_header = (emd_skel_header_t *) src_skel;
	emd_skel_relpos = (emd_vertex3_t *) (&src_skel[sizeof(emd_skel_header_t)]);
	emd_skel_data = (emd_armature_header_t *) (&src_skel[SDL_SwapLE16(emd_skel_header->relpos_len)]);

	/* Armature */
	emd1AddArmature(root, emd_skel_data, emd_skel_relpos, 0);

	/* Armature movement */
	src_move = &src_skel[SDL_SwapLE16(emd_skel_header->move_offset)];

	node = xmlNewNode(NULL, BAD_CAST "skel_move");
	xmlAddChild(root, node);
	for (i=0; i<emd1GetNumMovements(src, srcLen); i++) {
		xmlNodePtr node_move;
		emd1_skel_anim_t *emd_skel_anim = (emd1_skel_anim_t *) src_move;
		Sint16 *mesh_move = (Sint16 *) &src_move[sizeof(emd1_skel_anim_t)];

		node_move = xmlNewNode(NULL, BAD_CAST "movement");
		xmlAddChild(node, node_move);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node_move, BAD_CAST "id", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->pos.x));
		xmlNewProp(node_move, BAD_CAST "x", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->pos.y));
		xmlNewProp(node_move, BAD_CAST "y", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->pos.z));
		xmlNewProp(node_move, BAD_CAST "z", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->speed.x));
		xmlNewProp(node_move, BAD_CAST "dx", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->speed.y));
		xmlNewProp(node_move, BAD_CAST "dy", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->speed.z));
		xmlNewProp(node_move, BAD_CAST "dz", buf);

		for (j=0; j<SDL_SwapLE16(emd_skel_header->count); j++) {
			xmlNodePtr node_mesh;

			node_mesh = xmlNewNode(NULL, BAD_CAST "mesh_move");
			xmlAddChild(node_move, node_mesh);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", j);
			xmlNewProp(node_mesh, BAD_CAST "id", buf);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(mesh_move[0]));
			xmlNewProp(node_mesh, BAD_CAST "ax", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(mesh_move[1]));
			xmlNewProp(node_mesh, BAD_CAST "ay", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(mesh_move[2]));
			xmlNewProp(node_mesh, BAD_CAST "az", buf);

			mesh_move += 3;
		}

		/* Next movement */
		src_move += SDL_SwapLE16(emd_skel_header->move_size);
	}
}

void emd1AddArmature(xmlNodePtr root, emd_armature_header_t *emd_skel_data, emd_vertex3_t *emd_skel_relpos, int start_mesh)
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
	xmlNewProp(node, BAD_CAST "rx", buf);
	xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_relpos[start_mesh].y));
	xmlNewProp(node, BAD_CAST "ry", buf);
	xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_relpos[start_mesh].z));
	xmlNewProp(node, BAD_CAST "rz", buf);

	for (i=0; i<num_mesh; i++) {
		emd1AddArmature(node, emd_skel_data, emd_skel_relpos, armature[offset+i]);
	}
}

Uint32 emd1GetNumMovements(Uint8 *src, Uint32 srcLen)
{
	emd1_directory_t *emd1_dir = (emd1_directory_t *) (&src[srcLen-sizeof(emd1_directory_t)]);
	emd1_anim_header_t *anim_hdr = (emd1_anim_header_t *) &src[SDL_SwapLE32(emd1_dir->animation)];
	int i, j, num_seq = SDL_SwapLE16(anim_hdr[0].offset)/sizeof(emd1_anim_header_t);
	Uint32 *anim_frames = (Uint32 *) anim_hdr;
	Uint32 num_moves = 0;

	for (i=0; i<num_seq; i++) {
		Uint16 anim_offset = anim_hdr[i].offset;

		for (j=0; j<SDL_SwapLE16(anim_hdr[i].count); j++) {
			Uint32 frame = SDL_SwapLE32(anim_frames[(anim_hdr[i].offset>>2) + j]);

			if ((frame & 0xffffUL) > num_moves) {
				num_moves = frame & 0xffffUL;
			}
		}
	}

	return num_moves+1;
}

void emd1AddAnimation(Uint8 *src, Uint32 srcLen, xmlNodePtr root)
{
	emd1_directory_t *emd1_dir = (emd1_directory_t *) (&src[srcLen-sizeof(emd1_directory_t)]);
	emd1_anim_header_t *anim_hdr = (emd1_anim_header_t *) &src[SDL_SwapLE32(emd1_dir->animation)];
	xmlNodePtr node, node_frame;
	xmlChar buf[32];
	int i, j, num_seq = SDL_SwapLE16(anim_hdr[0].offset)/sizeof(emd1_anim_header_t);
	Uint32 *anim_frames = (Uint32 *) anim_hdr;

	for (i=0; i<num_seq; i++) {
		Uint16 anim_offset = SDL_SwapLE16(anim_hdr[i].offset);

		node = xmlNewNode(NULL, BAD_CAST "sequence");
		xmlAddChild(root, node);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		for (j=0; j<SDL_SwapLE16(anim_hdr[i].count); j++) {
			Uint32 frame = SDL_SwapLE32(anim_frames[(anim_offset>>2) + j]);

			node_frame = xmlNewNode(NULL, BAD_CAST "frame");
			xmlAddChild(node, node_frame);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", frame & 0xffff);
			xmlNewProp(node_frame, BAD_CAST "movement", buf);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", (frame>>16) & 0xffff);
			xmlNewProp(node_frame, BAD_CAST "flags", buf);
		}
	}
}

void emd1AddModel(Uint8 *src, Uint32 srcLen, xmlNodePtr root)
{
	xmlNodePtr node, node_vtx, node_nor, node_tri, node_tex, node_v;
	xmlChar buf[32];
	emd1_directory_t *emd1_dir = (emd1_directory_t *) (&src[srcLen-sizeof(emd1_directory_t)]);
	emd1_model_header_t *model_hdr = (emd1_model_header_t *) &src[SDL_SwapLE32(emd1_dir->model)];
	Uint8 *tmp = (Uint8 *) model_hdr;
	emd1_model_mesh_t *mesh = (emd1_model_mesh_t *) &tmp[sizeof(emd1_model_header_t)];
	int i, j;

	for (i=0; i<SDL_SwapLE32(model_hdr->count); i++) {
		emd1_model_triangle_t *tri;

		node = xmlNewNode(NULL, BAD_CAST "mesh");
		xmlAddChild(root, node);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		/* Vertices */
		emd1AddModelVertices((emd_vertex4_t *) &tmp[SDL_SwapLE32(mesh[i].vtx_offset)],
			SDL_SwapLE32(mesh[i].vtx_count), node);

		/* Normals */
		emd1AddModelNormals((emd_vertex4_t *) &tmp[SDL_SwapLE32(mesh[i].nor_offset)],
			SDL_SwapLE32(mesh[i].nor_count), node);

		/* Triangles */
		tmp = (Uint8 *) mesh;
		tri = (emd1_model_triangle_t *) &tmp[SDL_SwapLE32(mesh[i].tri_offset)];
		for (j=0; j<SDL_SwapLE32(mesh[i].tri_count); j++) {
			node_tri = xmlNewNode(NULL, BAD_CAST "triangle");
			xmlAddChild(node, node_tri);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", j);
			xmlNewProp(node_tri, BAD_CAST "id", buf);

			node_tex = xmlNewNode(NULL, BAD_CAST "texture");
			xmlAddChild(node_tri, node_tex);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", (SDL_SwapLE16(tri[j].page)<<1) & 0xff);
			xmlNewProp(node_tex, BAD_CAST "page", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[j].clutid) & 3);
			xmlNewProp(node_tex, BAD_CAST "clut", buf);

			node_v = xmlNewNode(NULL, BAD_CAST "vtx");
			xmlAddChild(node_tri, node_v);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[j].v0));
			xmlNewProp(node_v, BAD_CAST "v", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[j].n0));
			xmlNewProp(node_v, BAD_CAST "n", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri[j].tu0);
			xmlNewProp(node_v, BAD_CAST "tu", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri[j].tv0);
			xmlNewProp(node_v, BAD_CAST "tv", buf);

			node_v = xmlNewNode(NULL, BAD_CAST "vtx");
			xmlAddChild(node_tri, node_v);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[j].v1));
			xmlNewProp(node_v, BAD_CAST "v", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[j].n1));
			xmlNewProp(node_v, BAD_CAST "n", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri[j].tu1);
			xmlNewProp(node_v, BAD_CAST "tu", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri[j].tv1);
			xmlNewProp(node_v, BAD_CAST "tv", buf);

			node_v = xmlNewNode(NULL, BAD_CAST "vtx");
			xmlAddChild(node_tri, node_v);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[j].v2));
			xmlNewProp(node_v, BAD_CAST "v", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[j].n2));
			xmlNewProp(node_v, BAD_CAST "n", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri[j].tu2);
			xmlNewProp(node_v, BAD_CAST "tu", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri[j].tv2);
			xmlNewProp(node_v, BAD_CAST "tv", buf);
		}
	}
}

void emd1AddModelVertices(emd_vertex4_t *vtx, Uint32 count, xmlNodePtr root)
{
	int i;
	xmlNodePtr node;
	xmlChar buf[32];

	for (i=0; i<count; i++) {

		node = xmlNewNode(NULL, BAD_CAST "vertex");
		xmlAddChild(root, node);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(vtx[i].x));
		xmlNewProp(node, BAD_CAST "x", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(vtx[i].y));
		xmlNewProp(node, BAD_CAST "y", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(vtx[i].z));
		xmlNewProp(node, BAD_CAST "z", buf);
	}
}

void emd1AddModelNormals(emd_vertex4_t *nor, Uint32 count, xmlNodePtr root)
{
	int i;
	xmlNodePtr node;
	xmlChar buf[32];

	for (i=0; i<count; i++) {

		node = xmlNewNode(NULL, BAD_CAST "normal");
		xmlAddChild(root, node);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(nor[i].x));
		xmlNewProp(node, BAD_CAST "x", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(nor[i].y));
		xmlNewProp(node, BAD_CAST "y", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(nor[i].z));
		xmlNewProp(node, BAD_CAST "z", buf);
	}
}

void emd1AddTim(Uint8 *src, Uint32 srcLen, xmlNodePtr root, const char *filename)
{
	emd1_directory_t *emd1_dir = (emd1_directory_t *) (&src[srcLen-sizeof(emd1_directory_t)]);
	Uint32 tim_size = srcLen-sizeof(emd1_directory_t)-SDL_SwapLE32(emd1_dir->tim);
	char *dst_filename;

	dst_filename = get_filename_ext(filename, ".tim");
	if (!dst_filename) {
		return;
	}

	save_file(dst_filename, &src[SDL_SwapLE32(emd1_dir->tim)], tim_size);

	xmlNewProp(root, BAD_CAST "filename", dst_filename);

	free(dst_filename);
}

/*--- RE2 EMD ---*/

int emd2ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc)
{
	xmlNodePtr root, node;
	xmlChar buf[32];
	int i;

	printf("Detected RE2 EMD file\n");
	root = xmlNewNode(NULL, BAD_CAST "emd");	
	xmlNewProp(root, BAD_CAST "version", BAD_CAST "2");
	xmlDocSetRootElement(doc, root);

	for (i=0; i<1; i++) {
		/* Animations */
		node = xmlNewNode(NULL, BAD_CAST "animation");
		xmlAddChild(root, node);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);
		emd2AddAnimation(src, srcLen, i, node);

		/* Skeleton */
		node = xmlNewNode(NULL, BAD_CAST "skeleton");
		xmlAddChild(root, node);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);
		emd2AddSkeleton(src, srcLen, i, node);
	}

	/* Model */
	node = xmlNewNode(NULL, BAD_CAST "model");
	xmlAddChild(root, node);
	emd2AddModel(src, srcLen, node);

	return 0;
}

void emd2AddSkeleton(Uint8 *src, Uint32 srcLen, int num_skel, xmlNodePtr root)
{
	emd_header_t *emd_hdr;
	emd2_directory_t *emd2_dir;
	emd_skel_header_t *emd_skel_header;
	Uint8 *src_skel, *src_move;
	emd_vertex3_t *emd_skel_relpos;
	emd_armature_header_t *emd_skel_data;
	xmlNodePtr node;
	int i,j;

	emd_hdr = (emd_header_t *) src;
	emd2_dir = (emd2_directory_t *) &src[SDL_SwapLE32(emd_hdr->offset)];
	switch(num_skel) {
		case 1:
			emd_skel_header = (emd_skel_header_t *) &src[SDL_SwapLE32(emd2_dir->skeleton1)];
			break;
		case 2:
			emd_skel_header = (emd_skel_header_t *) &src[SDL_SwapLE32(emd2_dir->skeleton2)];
			break;
		case 0:
		default:
			emd_skel_header = (emd_skel_header_t *) &src[SDL_SwapLE32(emd2_dir->skeleton0)];
			break;
	}

	/*--- Skeleton ---*/
	src_skel = (Uint8 *) emd_skel_header;
	emd_skel_relpos = (emd_vertex3_t *) (&src_skel[sizeof(emd_skel_header_t)]);
	emd_skel_data = (emd_armature_header_t *) (&src_skel[SDL_SwapLE16(emd_skel_header->relpos_len)]);

	/* Armature */
	emd1AddArmature(root, emd_skel_data, emd_skel_relpos, 0);

	/* Armature movement */
	src_move = &src_skel[SDL_SwapLE16(emd_skel_header->move_offset)];

	node = xmlNewNode(NULL, BAD_CAST "skel_move");
	xmlAddChild(root, node);
	for (i=0; i<emd1GetNumMovements(src, srcLen); i++) {
		xmlNodePtr node_move;
		xmlChar buf[32];
		emd2_skel_anim_t *emd_skel_anim = (emd2_skel_anim_t *) src_move;
		Uint8 *mesh_move = (Uint8 *) &src_move[sizeof(emd2_skel_anim_t)];
		int mesh_move_pos = 0;	/* index inside mesh_move array, in 12 bits units */

		node_move = xmlNewNode(NULL, BAD_CAST "movement");
		xmlAddChild(node, node_move);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node_move, BAD_CAST "id", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->pos.x));
		xmlNewProp(node_move, BAD_CAST "x", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->pos.y));
		xmlNewProp(node_move, BAD_CAST "y", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->pos.z));
		xmlNewProp(node_move, BAD_CAST "z", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->speed.x));
		xmlNewProp(node_move, BAD_CAST "dx", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->speed.y));
		xmlNewProp(node_move, BAD_CAST "dy", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(emd_skel_anim->speed.z));
		xmlNewProp(node_move, BAD_CAST "dz", buf);

		for (j=0; j<SDL_SwapLE16(emd_skel_header->count); j++) {
			xmlNodePtr node_mesh;

			node_mesh = xmlNewNode(NULL, BAD_CAST "mesh_move");
			xmlAddChild(node_move, node_mesh);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", j);
			xmlNewProp(node_mesh, BAD_CAST "id", buf);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", emd2Read12Bits(mesh_move, mesh_move_pos));
			xmlNewProp(node_mesh, BAD_CAST "ax", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", emd2Read12Bits(mesh_move, mesh_move_pos+1));
			xmlNewProp(node_mesh, BAD_CAST "ay", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", emd2Read12Bits(mesh_move, mesh_move_pos+2));
			xmlNewProp(node_mesh, BAD_CAST "az", buf);

			mesh_move += 3;
		}

		/* Next movement */
		src_move += SDL_SwapLE16(emd_skel_header->move_size);
	}
}

Sint16 emd2Read12Bits(Uint8 *array, int index)
{
	Sint16 val = 0;

	switch(index & 1) {
		case 0:	/* XX and -X */
			val= array[index]|(array[index+1]<<8);
			break;
		case 1: /* Y- and YY */
			val = (array[index]>>4)|(array[index+1]<<4);
			break;
	}
	val &= 0xfff;
	if (val & (1<<11)) {
		val |= 0xf000;
	}

	return val;
}

void emd2AddAnimation(Uint8 *src, Uint32 srcLen, int num_anim, xmlNodePtr root)
{
	emd_header_t *emd_hdr;
	emd2_directory_t *emd2_dir;
	emd2_anim_header_t *anim_hdr;
	xmlNodePtr node, node_frame;
	xmlChar buf[32];
	int i, j, num_seq;
	Uint32 *anim_frames;

	emd_hdr = (emd_header_t *) src;
	emd2_dir = (emd2_directory_t *) &src[SDL_SwapLE32(emd_hdr->offset)];
	switch(num_anim) {
		case 1:
			anim_hdr = (emd2_anim_header_t * ) &src[SDL_SwapLE32(emd2_dir->animation1)];
			break;
		case 2:
			anim_hdr = (emd2_anim_header_t * ) &src[SDL_SwapLE32(emd2_dir->animation2)];
			break;
		case 0:
		default:
			anim_hdr = (emd2_anim_header_t * ) &src[SDL_SwapLE32(emd2_dir->animation0)];
			break;
	}
	num_seq = SDL_SwapLE16(anim_hdr[0].offset)/sizeof(emd2_anim_header_t);
	anim_frames = (Uint32 *) anim_hdr;

	for (i=0; i<num_seq; i++) {
		Uint16 anim_offset = SDL_SwapLE16(anim_hdr[i].offset);

		node = xmlNewNode(NULL, BAD_CAST "sequence");
		xmlAddChild(root, node);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		for (j=0; j<SDL_SwapLE16(anim_hdr[i].count); j++) {
			Uint32 frame = SDL_SwapLE32(anim_frames[(anim_offset>>2) + j]);

			node_frame = xmlNewNode(NULL, BAD_CAST "frame");
			xmlAddChild(node, node_frame);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", frame & 0xfff);
			xmlNewProp(node_frame, BAD_CAST "movement", buf);

			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", (frame>>12) & 0xfffff);
			xmlNewProp(node_frame, BAD_CAST "flags", buf);
		}
	}
}

void emd2AddModel(Uint8 *src, Uint32 srcLen, xmlNodePtr root)
{
	xmlNodePtr node, node_vtx, node_nor, node_tri, node_tex, node_v;
	xmlChar buf[32];
	emd_header_t *emd_hdr;
	emd2_directory_t *emd2_dir;
	emd2_model_header_t *model_hdr;
	emd2_model_object_t *model_obj;
	Uint8 *tmp;
	int i;

	emd_hdr = (emd_header_t *) src;
	emd2_dir = (emd2_directory_t *) &src[SDL_SwapLE32(emd_hdr->offset)];
	model_hdr = (emd2_model_header_t *) &src[SDL_SwapLE32(emd2_dir->model)];
	tmp = (Uint8 *) model_hdr;
	model_obj = (emd2_model_object_t *) &tmp[sizeof(emd2_model_header_t)];

	for (i=0; i<SDL_SwapLE32(model_hdr->count)>>1; i++) {
		node = xmlNewNode(NULL, BAD_CAST "mesh");
		xmlAddChild(root, node);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		emd2AddModelTriangles((Uint8 *) model_obj, &(model_obj[i].triangle), node);
		emd2AddModelQuads((Uint8 *) model_obj, &(model_obj[i].quad), node);
	}
}

void emd2AddModelTriangles(Uint8 *src, emd2_model_triangle_t *model_tri, xmlNodePtr root)
{
	xmlNodePtr node;

	node = xmlNewNode(NULL, BAD_CAST "triangles");
	xmlAddChild(root, node);

	/* Vertices */
	emd1AddModelVertices((emd_vertex4_t *) &src[SDL_SwapLE32(model_tri->vtx_offset)],
		SDL_SwapLE32(model_tri->vtx_count), node);

	/* Normals */
	emd1AddModelNormals((emd_vertex4_t *) &src[SDL_SwapLE32(model_tri->nor_offset)],
		SDL_SwapLE32(model_tri->nor_count), node);

	/* Texture,Triangles */
	emd2AddModelTri((emd2_triangle_t *) &src[SDL_SwapLE32(model_tri->tri_offset)],
		(emd2_triangle_tex_t *) &src[SDL_SwapLE32(model_tri->tex_offset)],
		SDL_SwapLE32(model_tri->tri_count), node);
}

void emd2AddModelQuads(Uint8 *src, emd2_model_quad_t *model_quad, xmlNodePtr root)
{
	xmlNodePtr node;

	node = xmlNewNode(NULL, BAD_CAST "quads");
	xmlAddChild(root, node);

	/* Vertices */
	emd1AddModelVertices((emd_vertex4_t *) &src[SDL_SwapLE32(model_quad->vtx_offset)],
		SDL_SwapLE32(model_quad->vtx_count), node);

	/* Normals */
	emd1AddModelNormals((emd_vertex4_t *) &src[SDL_SwapLE32(model_quad->nor_offset)],
		SDL_SwapLE32(model_quad->nor_count), node);

	/* Texture,Quads */
	emd2AddModelQuad((emd2_quad_t *) &src[SDL_SwapLE32(model_quad->quad_offset)],
		(emd2_quad_tex_t *) &src[SDL_SwapLE32(model_quad->tex_offset)],
		SDL_SwapLE32(model_quad->quad_count), node);
}

void emd2AddModelTri(emd2_triangle_t *tri, emd2_triangle_tex_t *tri_tex, Uint32 count, xmlNodePtr root)
{
	int i, j;
	xmlNodePtr node, node_tx, node_v;
	xmlChar buf[32];

	for (i=0; i<count; i++) {
		node = xmlNewNode(NULL, BAD_CAST "triangle");
		xmlAddChild(root, node);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		/* Texture */
		node_tx = xmlNewNode(NULL, BAD_CAST "texture");
		xmlAddChild(node, node_tx);

		/*xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node_tx, BAD_CAST "id", buf);*/

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", (SDL_SwapLE16(tri_tex[i].page)<<1) & 0xff);
		xmlNewProp(node_tx, BAD_CAST "page", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri_tex[i].clutid) & 3);
		xmlNewProp(node_tx, BAD_CAST "clut", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri_tex[i].tu0);
		xmlNewProp(node_tx, BAD_CAST "tu0", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri_tex[i].tv0);
		xmlNewProp(node_tx, BAD_CAST "tv0", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri_tex[i].tu1);
		xmlNewProp(node_tx, BAD_CAST "tu1", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri_tex[i].tv1);
		xmlNewProp(node_tx, BAD_CAST "tv1", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri_tex[i].tu2);
		xmlNewProp(node_tx, BAD_CAST "tu2", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", tri_tex[i].tv2);
		xmlNewProp(node_tx, BAD_CAST "tv2", buf);

		/* Vertices */
		for (j=0; j<3; j++) {
			node_v = xmlNewNode(NULL, BAD_CAST "vtx");
			xmlAddChild(node, node_v);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[i].vtx[j].v));
			xmlNewProp(node_v, BAD_CAST "v", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(tri[i].vtx[j].n));
			xmlNewProp(node_v, BAD_CAST "n", buf);
		}
	}
}

void emd2AddModelQuad(emd2_quad_t *quad, emd2_quad_tex_t *quad_tex, Uint32 count, xmlNodePtr root)
{
	int i, j;
	xmlNodePtr node, node_tx, node_v;
	xmlChar buf[32];

	for (i=0; i<count; i++) {
		node = xmlNewNode(NULL, BAD_CAST "quad");
		xmlAddChild(root, node);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node, BAD_CAST "id", buf);

		/* Texture */
		node_tx = xmlNewNode(NULL, BAD_CAST "texture");
		xmlAddChild(node, node_tx);

		/*xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", i);
		xmlNewProp(node_tx, BAD_CAST "id", buf);*/

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", (SDL_SwapLE16(quad_tex[i].page)<<1) & 0xff);
		xmlNewProp(node_tx, BAD_CAST "page", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(quad_tex[i].clutid) & 3);
		xmlNewProp(node_tx, BAD_CAST "clut", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tu0);
		xmlNewProp(node_tx, BAD_CAST "tu0", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tv0);
		xmlNewProp(node_tx, BAD_CAST "tv0", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tu1);
		xmlNewProp(node_tx, BAD_CAST "tu1", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tv1);
		xmlNewProp(node_tx, BAD_CAST "tv1", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tu2);
		xmlNewProp(node_tx, BAD_CAST "tu2", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tv2);
		xmlNewProp(node_tx, BAD_CAST "tv2", buf);

		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tu3);
		xmlNewProp(node_tx, BAD_CAST "tu3", buf);
		xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", quad_tex[i].tv3);
		xmlNewProp(node_tx, BAD_CAST "tv3", buf);

		/* Vertices */
		for (j=0; j<4; j++) {
			node_v = xmlNewNode(NULL, BAD_CAST "vtx");
			xmlAddChild(node, node_v);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(quad[i].vtx[j].v));
			xmlNewProp(node_v, BAD_CAST "v", buf);
			xmlStrPrintf(buf, sizeof(buf), BAD_CAST "%d", SDL_SwapLE16(quad[i].vtx[j].n));
			xmlNewProp(node_v, BAD_CAST "n", buf);
		}
	}
}

/*--- RE3 EMD ---*/

int emd3ToXml(Uint8 *src, Uint32 srcLen, xmlDoc *doc)
{
	printf("Detected RE3 EMD file\n");
	return 1;
}
