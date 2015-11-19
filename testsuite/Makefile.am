noinst_PROGRAMS = vc2encodertest

TESTS = vc2encodertest

AM_CFLAGS = $(VC2HQDECODE_CFLAGS) \
	$(BOOST_CFLAGS)

AM_CXXFLAGS = $(VC2HQENCODE_CFLAGS) \
	$(BOOST_CFLAGS) \
	$(BOOST_CPPFLAGS) \
	-I$(top_srcdir)/vc2hqencode

AM_LDFLAGS = $(VC2HQENCODE_LDFLAGS)

LDADD = $(VC2HQENCODE_LIBS) \
	$(BOOST_SYSTEM_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_PROGRAM_OPTIONS_LIB)

vc2encodertest_SOURCES = \
	tests.cpp \
	test_transforms.cpp
