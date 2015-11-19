noinst_LTLIBRARIES = libvc2transform-avx2.la

$(top_builddir)/vc2transform_avx2/haar_transform_sse4_2.hpp : $(top_srcdir)/vc2transform_sse4_2/haar_transform.hpp
	$(top_srcdir)/duplicate-transform $< $@ sse4_2 sse4_2_avx2

$(top_builddir)/vc2transform_avx2/legall_transform_sse4_2.hpp : $(top_srcdir)/vc2transform_sse4_2/legall_transform.hpp
	$(top_srcdir)/duplicate-transform $< $@ sse4_2 sse4_2_avx2

libvc2transform_avx2_la_LDFLAGS = \
	-no-undefined \
	-version-info $(VC2HQENCODE_LIBVERSION) \
	$(VC2HQENCODE_LDFLAGS) \
	-lpthread

libvc2transform_avx2_la_CPPFLAGS = $(VC2HQENCODE_CFLAGS) \
	$(AVX2_FLAGS) \
	-I$(top_srcdir)/vc2hqencode

libvc2transform_avx2_la_SOURCES = \
	transform_avx2.cpp

noinst_HEADERS = \
        transform_avx2.hpp \
        transform_kernels.hpp \
        haar_transform.hpp

BUILT_SOURCES = \
	$(top_builddir)/vc2transform_avx2/haar_transform_sse4_2.hpp \
	$(top_builddir)/vc2transform_avx2/legall_transform_sse4_2.hpp

CLEANFILES = \
	$(top_builddir)/vc2transform_avx2/haar_transform_sse4_2.hpp \
	$(top_builddir)/vc2transform_avx2/legall_transform_sse4_2.hpp
