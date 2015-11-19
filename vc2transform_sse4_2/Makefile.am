noinst_LTLIBRARIES = libvc2transform-sse4-2.la

libvc2transform_sse4_2_la_LDFLAGS = \
	-no-undefined \
	-version-info $(VC2HQENCODE_LIBVERSION) \
	$(VC2HQENCODE_LDFLAGS) \
	-lpthread

libvc2transform_sse4_2_la_CPPFLAGS = $(VC2HQENCODE_CFLAGS) \
	$(SSE4_2_FLAGS) \
	-I$(top_srcdir)/vc2hqencode

libvc2transform_sse4_2_la_SOURCES = \
	transform_sse4_2.cpp

noinst_HEADERS = \
        transform_sse4_2.hpp \
        transform_kernels.hpp \
        legall_transform.hpp \
        haar_transform.hpp
