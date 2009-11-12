bin_PROGRAMS = adt2img bss2bmp pak2tim pix2bmp

AM_CFLAGS = $(SDL_CFLAGS)

LIBS = $(SDL_LIBS)

common_headers = file_functions.h

adt2img_SOURCES = adt2img.c file_functions.c depack_adt.c

adt2img_headers = depack_adt.h

bss2bmp_SOURCES = bss2bmp.c depack_mdec.c depack_vlc.c idctfst.c

bss2bmp_headers = depack_mdec.h depack_vlc.h idctfst.h

pak2tim_SOURCES = pak2tim.c file_functions.c depack_pak.c

pak2tim_headers = depack_pak.h

pix2bmp_SOURCES = pix2bmp.c

EXTRA_DIST = $(common_headers) $(adt2img_headers) $(bss2bmp_headers) \
	$(pak2tim_headers)
