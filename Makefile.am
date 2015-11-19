
AUTOMAKE_OPTIONS = foreign

include doxygen.am

SUBDIRS = vc2transform_c vc2transform_sse4_2 vc2transform_avx vc2transform_avx2 vc2hqencode testprogs testsuite redist

EXTRA_DIST = COPYING autogen.sh duplicate-transform

pkgconfig_DATA = vc2hqencode-$(VC2HQENCODE_MAJORMINOR).pc

vc2hqencode-$(VC2HQENCODE_MAJORMINOR).pc: vc2hqencode.pc
	cp vc2hqencode.pc vc2hqencode-$(VC2HQENCODE_MAJORMINOR).pc

vc2hqencode-$(VC2HQENCODE_MAJORMINOR)-uninstalled.pc: vc2hqencode-uninstalled.pc
	cp vc2hqencode-uninstalled.pc vc2hqencode-$(VC2HQENCODE_MAJORMINOR)-uninstalled.pc

BUILT_SOURCES = vc2hqencode-$(VC2HQENCODE_MAJORMINOR)-uninstalled.pc

CLEANFILES = vc2hqencode-$(VC2HQENCODE_MAJORMINOR).pc vc2hqencode-$(VC2HQENCODE_MAJORMINOR)-uninstalled.pc

ACLOCAL_FLAGS = -I m4
