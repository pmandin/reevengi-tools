/*
	File loader/saver

	Copyright (C) 2009	Patrice Mandin

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

#ifndef FILE_FUNCTIONS_H
#define FILE_FUNCTIONS_H 1

void save_file(const char *filename, void *buffer, int length);

void save_bmp(const char *src_filename, SDL_Surface *image);

void save_tim(const char *src_filename, Uint8 *buffer, int length);

void save_pak(const char *src_filename, Uint8 *buffer, int length);

void save_raw(const char *src_filename, Uint8 *buffer, int length);

#endif /* FILE_FUNCTIONS_H */
