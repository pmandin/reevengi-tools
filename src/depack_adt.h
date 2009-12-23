/*
	ADT file depacker

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

#ifndef DEPACK_ADT_H
#define DEPACK_ADT_H

/*
	Depack an ADT file

	src		Source file
	dstPointer	Pointer to depacked file buffer (NULL if failed)
	dstLength	Length of depacked file (0 if failed)
*/
void adt_depack(SDL_RWops *src, Uint8 **dstPointer, int *dstLength);

/*
	Create a SDL_Surface, for a depacked ADT file
	source		Pointer to depacked file
	reorganize	0: keep buffer as is
			1: buffer is organized with 256x256 block first, then 2 64x128
*/
SDL_Surface *adt_surface(Uint16 *source, int reorganize);

#endif /* DEPACK_ADT_H */
