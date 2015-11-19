noinst_LTLIBRARIES = libvc2transform-c.la

libvc2transform_c_la_LDFLAGS = \
	-no-undefined \
	-version-info $(VC2HQENCODE_LIBVERSION) \
	$(VC2HQENCODE_LDFLAGS) \
	-lpthread

libvc2transform_c_la_CPPFLAGS = $(VC2HQENCODE_CFLAGS) \
	-I$(top_srcdir)/vc2hqencode

libvc2transform_c_la_SOURCES = \
	transform_c.cpp

noinst_HEADERS = \
        transform_c.hpp \
        transform_kernels.hpp \
        legall_transform.hpp \
        haar_transform.hpp \
        deslauriers_dubuc_9_7_transform.hpp \
        deslauriers_dubuc_13_7_transform.hpp \
        fidelity_transform.hpp
