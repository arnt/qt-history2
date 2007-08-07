## Process this file with automake to produce Makefile.in

INCLUDES = 					\
	$(FREETYPE_CFLAGS)

noinst_LTLIBRARIES = libharfbuzz-1.la

MAINSOURCES =  \
	ftglue.c \
	harfbuzz-buffer.c \
	harfbuzz-dump.c \
	harfbuzz-gdef.c \
	harfbuzz-gpos.c \
	harfbuzz-gsub.c \
	harfbuzz-open.c \
	harfbuzz-shaper.cpp \
	harfbuzz-tibetan.cpp \
	harfbuzz-myanmar.cpp \
	harfbuzz-khmer.cpp \
	harfbuzz-indic.cpp \
	harfbuzz-hebrew.cpp \
	harfbuzz-arabic.cpp \
	harfbuzz-hangul.cpp \
	harfbuzz-myanmar.cpp

EXTRA_SOURCES = harfbuzz.c

PUBLICHEADERS = \
	harfbuzz.h \
	harfbuzz-buffer.h \
	harfbuzz-dump.h \
	harfbuzz-gdef.h \
	harfbuzz-gpos.h \
	harfbuzz-gsub.h \
	harfbuzz-open.h \
	harfbuzz-shaper.h

PRIVATEHEADERS = \
	ftglue.h \
	harfbuzz-impl.h \
	harfbuzz-gdef-private.h \
	harfbuzz-gpos-private.h \
	harfbuzz-gsub-private.h \
	harfbuzz-open-private.h \
	harfbuzz-shaper-private.h

libharfbuzz_1_la_SOURCES = \
	$(MAINSOURCES) \
	$(PUBLICHEADERS) \
	$(PRIVATEHEADERS)

libharfbuzz_1_la_LIBADD = \
	$(FREETYPE_LIBS)

#noinst_PROGRAMS = harfbuzz-dump
#
#harfbuzz_dump_SOURCES = 	\
#	harfbuzz-dump-main.c
#
#harfbuzz_dump_LDADD = 		\
#	libharfbuzz-1.la

EXTRA_DIST = 		\
	README		\
	COPYING.FTL	\
	COPYING.GPL	\
	COPYING		\
	$(EXTRA_SOURCES)

