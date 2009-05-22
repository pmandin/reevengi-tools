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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <sys/stat.h>
#include <io.h>
#endif

char *load_file(const char *filename, int *length)
{
	int handle;
	char *buffer;

	/* Load file */
#ifdef WIN32
	handle = _open(filename, O_RDONLY|_O_BINARY);
#else
	handle = open(filename, O_RDONLY);
#endif
	if (handle<0) {
		fprintf(stderr, "Unable to open %s\n", filename);	
		return NULL;
	}

#ifdef WIN32
	*length = _lseek(handle, 0, SEEK_END);
	_lseek(handle, 0, SEEK_SET); 	
#else
	*length = lseek(handle, 0, SEEK_END);
	lseek(handle, 0, SEEK_SET); 	
#endif

	buffer = (char *)malloc(*length);
	if (buffer==NULL) {
		fprintf(stderr, "Unable to allocate %d bytes\n", *length);
		return NULL;
	}

#ifdef WIN32
	_read(handle, buffer, *length);
	_close(handle);
#else
	read(handle, buffer, *length);
	close(handle);
#endif

	return buffer;
}

void save_file(const char *filename, void *buffer, int length)
{
	int handle;

	/* Load file */
#ifdef WIN32
	handle = _open(filename, _O_CREAT | _O_TRUNC | _O_BINARY | _O_RDWR, _S_IREAD | _S_IWRITE);
#else
	handle = creat(filename, 0664);
#endif
	if (handle<0) {
		fprintf(stderr, "Unable to open %s\n", filename);	
		return;
	}

#ifdef WIN32
	_write(handle, buffer, length);
	_close(handle);
#else
	write(handle, buffer, length);
	close(handle);
#endif
}
