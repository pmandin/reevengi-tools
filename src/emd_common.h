/*
	EMD common structures

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

#ifndef EMD_COMMON_H
#define EMD_COMMON_H 1

#include <SDL.h>

/*--- Structures ---*/

/* RE2, RE3 */
typedef struct {
	Uint32 offset;
	Uint32 length;
} emd_header_t;

/* All */
typedef struct {
	Sint16 x,y,z;
} emd_vertex3_t;

typedef struct {
	Sint16 x,y,z;
	Sint16 pad;
} emd_vertex4_t;

/* Skeleton */
typedef struct {
	Uint16	relpos_len;
	Uint16	move_offset;
	Uint16	count;
	Uint16	move_size;
} emd_skel_header_t;

typedef struct {
	Uint16	num_mesh;
	Uint16	offset;
} emd_armature_header_t;

#endif /* EMD_COMMON_H */
