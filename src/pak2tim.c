/*
	PAK file depacker

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "depack_pak.h"
#include "file_functions.h"

void save_tim(const char *src_filename, char *buffer, int length)
{
	int dst_namelength = strlen(src_filename)+1;
	char *dst_filename;
	char *posname, *posext;

	dst_filename = (char *) malloc(dst_namelength);
	if (!dst_filename) {
		fprintf(stderr, "Can not allocate %d bytes\n", dst_namelength);
		return;
	}

	posname = strrchr(src_filename, '/');
	if (posname) {
		++posname;	/* Go after / */
	} else {
		posname = strrchr(src_filename, '\\');
		if (posname) {
			++posname;	/* Go after \\ */
		} else {
			/* No directory in source filename */
			posname = src_filename;
		}
	}
	sprintf(dst_filename, "%s", posname);

	posext = strrchr(dst_filename, '.');
	if (!posext) {
		strcat(dst_filename, ".tim");
	} else {
		++posext;
		strcpy(posext, "tim");
	}

	save_file(dst_filename, buffer, length);

	free(dst_filename);
}

int main(int argc, char **argv)
{
	char *src_file, *dst_file;
	int src_length, dst_length;

	if (argc<2) {
		fprintf(stderr, "Usage: %s /path/to/filename.pak\n", argv[0]);
		return 1;
	}

	src_file = load_file(argv[1], &src_length);
	if (!src_file) {
		return 1;
	}

	pak_depack(src_file, src_length, &dst_file, &dst_length);

	if (dst_file && dst_length) {
		save_tim(argv[1], dst_file, dst_length);

		free(dst_file);
	} else {
		fprintf(stderr, "Error depacking file\n");
	}
	free(src_file);

	return 0;
}
