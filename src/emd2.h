/*
	RE2 EMD structures

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

#ifndef EMD2_H
#define EMD2_H 1

#include <SDL.h>

#include "emd_common.h"

/*--- Structures ---*/

/* Directory */
typedef struct {
	Uint32	unknown;
	Uint32	animation0;
	Uint32	skeleton0;
	Uint32	animation1;
	Uint32	skeleton1;
	Uint32	animation2;
	Uint32	skeleton2;
	Uint32	model;
} emd2_directory_t;

/* Skeleton */
typedef struct {
	emd_vertex3_t	pos;
	emd_vertex3_t	speed;

	/* 12 bits values for angles following */
} emd2_skel_anim_t;

/* Animation */
typedef struct {
	Uint16	count;
	Uint16	offset;
} emd2_anim_header_t;

/* Model */
typedef struct {
	Uint32	length;
	Uint32	unknown;
	Uint32	count;
} emd2_model_header_t;

typedef struct {
	Uint32	vtx_offset;
	Uint32	vtx_count;
	Uint32	nor_offset;
	Uint32	nor_count;
	Uint32	tri_offset;
	Uint32	tri_count;
	Uint32	tex_offset;
} emd2_model_triangle_t;

typedef struct {
	Uint32	vtx_offset;
	Uint32	vtx_count;
	Uint32	nor_offset;
	Uint32	nor_count;
	Uint32	quad_offset;
	Uint32	quad_count;
	Uint32	tex_offset;
} emd2_model_quad_t;

typedef struct {
	emd2_model_triangle_t triangle;
	emd2_model_quad_t quad;
} emd2_model_object_t;

typedef struct {
	Uint16	n;	/* Normal */
	Uint16	v;	/* Vertex */
} emd2_vtx_nv_t;

typedef struct {
	emd2_vtx_nv_t vtx[3];
} emd2_triangle_t;

typedef struct {
	Uint8 tu0,tv0;
	Uint16 clutid;
	Uint8 tu1,tv1;
	Uint16 page;
	Uint8 tu2,tv2;
	Uint16 dummy;
} emd2_triangle_tex_t;

typedef struct {
	emd2_vtx_nv_t vtx[4];
} emd2_quad_t;

typedef struct {
	Uint8 tu0,tv0;
	Uint16 clutid;
	Uint8 tu1,tv1;
	Uint16 page;
	Uint8 tu2,tv2;
	Uint16 dummy0;
	Uint8 tu3,tv3;
	Uint16 dummy1;
} emd2_quad_tex_t;

#endif /* EMD2_H */
