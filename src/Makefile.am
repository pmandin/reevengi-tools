bin_PROGRAMS = adt2img bss2bmp pak2tim pix2bmp ptc2bmp rgb2bmp rofs sld \
	extract_bin iso_search file2pak emd2xml

AM_CFLAGS = $(SDL_CFLAGS)

LIBS = $(SDL_LIBS)

common_headers = file_functions.h param.h

adt2img_SOURCES = adt2img.c file_functions.c depack_adt.c param.c

adt2img_headers = depack_adt.h

bss2bmp_SOURCES = bss2bmp.c depack_mdec.c depack_vlc.c idctfst.c \
	file_functions.c

bss2bmp_headers = depack_mdec.h depack_vlc.h idctfst.h

pak2tim_SOURCES = pak2tim.c file_functions.c depack_pak.c param.c

pak2tim_headers = depack_pak.h

pix2bmp_SOURCES = pix2bmp.c file_functions.c

ptc2bmp_SOURCES = ptc2bmp.c file_functions.c

rgb2bmp_SOURCES = rgb2bmp.c file_functions.c

file2pak_SOURCES = file2pak.c file_functions.c pack_pak.c param.c

file2pak_headers = pack_pak.h

rofs_SOURCES = rofs.c file_functions.c

sld_headers = depack_sld.h

sld_SOURCES = sld.c depack_sld.c file_functions.c

extract_bin_SOURCES = bin.c file_functions.c

iso_search_SOURCES = iso_search.c md5.c param.c

iso_search_headers = md5.h background_tim.h

emd2xml_SOURCES = emd2xml.c file_functions.c
emd2xml_CFLAGS = $(LIBXML_CFLAGS) $(AM_CFLAGS)
emd2xml_LDFLAGS = $(LIBXML_LIBS)

emd2xml_headers = emd_common.h

EXTRA_DIST = $(common_headers) $(adt2img_headers) $(bss2bmp_headers) \
	$(pak2tim_headers) $(tim2pak_headers) $(sld_headers) \
	$(iso_search_headers) $(file2pak_headers) $(emd2xml_headers)
