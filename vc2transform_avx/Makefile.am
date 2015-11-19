noinst_LTLIBRARIES = libvc2transform-avx.la

$(top_builddir)/vc2transform_avx/haar_transform_sse4_2.hpp : $(top_srcdir)/vc2transform_sse4_2/haar_transform.hpp
	$(top_srcdir)/duplicate-transform $< $@ sse4_2 sse4_2_avx

$(top_builddir)/vc2transform_avx/legall_transform_sse4_2.hpp : $(top_srcdir)/vc2transform_sse4_2/legall_transform.hpp
	$(top_srcdir)/duplicate-transform $< $@ sse4_2 sse4_2_avx

libvc2transform_avx_la_LDFLAGS = \
	-no-undefined \
	-version-info $(VC2HQENCODE_LIBVERSION) \
	$(VC2HQENCODE_LDFLAGS) \
	-lpthread

libvc2transform_avx_la_CPPFLAGS = $(VC2HQENCODE_CFLAGS) \
	$(AVX_FLAGS) \
	-I$(top_srcdir)/vc2hqencode \
	-I$(top_builddir)/vc2transform_avx

libvc2transform_avx_la_SOURCES = \
	transform_avx.cpp

noinst_HEADERS = \
        transform_avx.hpp \
        transform_kernels.hpp

BUILT_SOURCES = \
	$(top_builddir)/vc2transform_avx/haar_transform_sse4_2.hpp \
	$(top_builddir)/vc2transform_avx/legall_transform_sse4_2.hpp

CLEANFILES = \
	$(top_builddir)/vc2transform_avx/haar_transform_sse4_2.hpp \
	$(top_builddir)/vc2transform_avx/legall_transform_sse4_2.hpp
