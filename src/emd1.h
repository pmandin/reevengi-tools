/*
	RE1 EMD structures

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

#ifndef EMD1_H
#define EMD1_H 1

#include <SDL.h>

#include "emd_common.h"

/*--- Structures ---*/

/* Directory */
typedef struct {
	Uint32	skeleton;
	Uint32	animation;
	Uint32	model;
	Uint32	tim;
} emd1_directory_t;

/* Skeleton */
typedef struct {
	emd_vertex3_t	pos;
	emd_vertex3_t	speed;

	/* 16 bits values for angles following */
} emd1_skel_anim_t;

/* Animation */
typedef struct {
	Uint16	count;
	Uint16	offset;
} emd1_anim_header_t;

/* Model */
typedef struct {
	Uint32	length;
	Uint32	unknown;
	Uint32	count;
} emd1_model_header_t;

typedef struct {
	Uint32	vtx_offset;
	Uint32	vtx_count;
	Uint32	nor_offset;
	Uint32	nor_count;
	Uint32	tri_offset;
	Uint32	tri_count;
	Uint32	dummy;
} emd1_model_mesh_t;

typedef struct {
	Uint32 unknown;

	Uint8 tu0,tv0;
	Uint16 clutid;
	Uint8 tu1,tv1;
	Uint16 page;
	Uint8 tu2,tv2;
	Uint16 dummy;

	Uint16	n0,v0;
	Uint16	n1,v1;
	Uint16	n2,v2;
} emd1_model_triangle_t;

#endif /* EMD1_H */
