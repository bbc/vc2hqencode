noinst_PROGRAMS = vc2encode $(linux_only_progs)

AM_CFLAGS = $(VC2HQENCODE_CFLAGS)

AM_CXXFLAGS = $(VC2HQENCODE_CFLAGS)

AM_LDFLAGS = $(VC2HQENCODE_LDFLAGS)

LDADD = $(VC2HQENCODE_LIBS)

vc2encode_SOURCES = vc2encode.cpp \
	movingzoneplate.cpp \
	program_options_lite.cpp

noinst_HEADERS = program_options_lite.h
