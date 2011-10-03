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

/*--- Defines ---*/

#define EMD1_SKELETON 0
#define EMD1_ANIMATION 1
#define EMD1_MESH 2
#define EMD1_TIM 3

/*--- Structures ---*/

/* Skeleton */
typedef struct {
	Uint16	num_mesh;
	Uint16	offset;
} emd1_skel_data_t;

typedef struct {
	emd_vertex3_t	pos;
	emd_vertex3_t	speed;

	/* 16 bits values for angles following */
} emd1_skel_anim_t;

/* Animation */

/* Mesh */

#endif /* EMD1_H */
