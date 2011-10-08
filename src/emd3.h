/*
	RE3 EMD structures

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

#ifndef EMD3_H
#define EMD3_H 1

#include <SDL.h>

#include "emd_common.h"

/*--- Structures ---*/

/* Directory */
typedef struct {
	Uint32	unknown0[2];
	Uint32	animation0;
	Uint32	skeleton0;
	Uint32	animation1;
	Uint32	skeleton1;
	Uint32	animation2;
	Uint32	skeleton2;
	Uint32	unknown1[6];
	Uint32	model;
} emd3_directory_t;

/* Animation */
typedef struct {
	Uint16	count;
	Uint16	offset;
	Uint16	pad[2];
} emd3_anim_header_t;

/* Skeleton */
typedef struct {
	emd_vertex3_t	pos;
	Sint16	speed_y;

	/* 12 bits values for angles following */
} emd3_skel_anim_t;

/* Model */
typedef struct {
	Uint32	length;
	Uint32	count;
} emd3_model_header_t;

typedef struct {
	Uint32	vtx_offset;
	Uint32	nor_offset;
	Uint32	vtx_count;
	Uint32	tri_offset;
	Uint32	quad_offset;
	Uint16	tri_count;
	Uint16	quad_count;
} emd3_model_object_t;

typedef struct {
	Uint8 tu0,tv0;
	Uint8 page,dummy0;
	Uint8 tu1,tv1;
	Uint8 clutid,v0;
	Uint8 tu2,tv2;
	Uint8 v1,v2;
} emd3_triangle_t;

typedef struct {
	Uint8 tu0,tv0;
	Uint8 page,dummy0;
	Uint8 tu1,tv1;
	Uint8 clutid,dummy1;
	Uint8 tu2,tv2;
	Uint8 v0,v1;
	Uint8 tu3,tv3;
	Uint8 v2,v3;
} emd3_quad_t;

#endif /* EMD3_H */
