configfile_DATA =

pkgincludedir = $(includedir)/vc2hqencode-@VC2HQENCODE_MAJORMINOR@/vc2hqencode

lib_LTLIBRARIES = libvc2hqencode-@VC2HQENCODE_MAJORMINOR@.la

libvc2hqencode_@VC2HQENCODE_MAJORMINOR@_la_LIBADD = \
  $(BOOST_LDFLAGS) \
  $(BOOST_SYSTEM_LIB)\
  $(BOOST_THREAD_LIB) \
	$(RT_LIBS) \
	$(NUMA_LIBS) \
	$(top_builddir)/vc2transform_c/libvc2transform-c.la \
	$(top_builddir)/vc2transform_sse4_2/libvc2transform-sse4-2.la \
	$(top_builddir)/vc2transform_avx/libvc2transform-avx.la \
	$(top_builddir)/vc2transform_avx2/libvc2transform-avx2.la

libvc2hqencode_@VC2HQENCODE_MAJORMINOR@_la_LDFLAGS = \
	-no-undefined \
	-version-info $(VC2HQENCODE_LIBVERSION) \
	$(VC2HQENCODE_LDFLAGS) \
	-lpthread

libvc2hqencode_@VC2HQENCODE_MAJORMINOR@_la_CPPFLAGS = $(VC2HQENCODE_CFLAGS) \
	$(BOOST_CFLAGS) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/redist

libvc2hqencode_@VC2HQENCODE_MAJORMINOR@_la_SOURCES = vc2hqencode.cpp \
	VC2Encoder.cpp \
	logger.cpp \
	quantise.cpp \
	encode.cpp \
	stats.cpp \
	stream.cpp

pkginclude_HEADERS = \
	vc2hqencode.h

noinst_HEADERS = VC2Encoder.hpp \
	transform.hpp \
  VideoFormat.hpp \
  quantise.hpp \
  logger.hpp \
	thread_group.hpp \
  ThreadPool.hpp \
  datastructures.hpp \
  encode.hpp \
  lut.hpp \
  serialise.hpp \
  stats.hpp \
  stream.hpp \
  intdivide.hpp \
  debug.hpp \
  quantiserselection.hpp \
	encode_slice_component_optimised.hpp

nodist_pkginclude_HEADERS = vc2hqencode-stdint.h

DISTCLEANFILES = vc2hqencode-stdint.h
