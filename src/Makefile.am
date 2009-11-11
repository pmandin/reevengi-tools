bin_PROGRAMS = pak2tim bss2bmp

AM_CFLAGS = $(SDL_CFLAGS)

LIBS = $(SDL_LIBS)

pak2tim_SOURCES = pak2tim.c file_functions.c depack_pak.c

pak2tim_headers = depack_pak.h

bss2bmp_SOURCES = bss2bmp.c depack_mdec.c depack_vlc.c idctfst.c

bss2bmp_headers = depack_mdec.h depack_vlc.h idctfst.h

EXTRA_DIST = $(pak2tim_headers)
