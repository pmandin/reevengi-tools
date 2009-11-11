/*
	MDEC depacker

	Copyright (C) 2007	Patrice Mandin

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

#ifndef DEPACK_MDEC_H
#define DEPACK_MDEC_H

void mdec_depack(SDL_RWops *src, Uint8 **dstPointer, int *dstLength,
	int width, int height);

SDL_Surface *mdec_surface(Uint8 *source, int width, int height, int row_offset);

#endif /* DEPACK_MDEC_H */
