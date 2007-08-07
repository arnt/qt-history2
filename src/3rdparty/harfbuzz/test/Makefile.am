INCLUDES = -I$(srcdir)/../src $(FONTCONFIG_CFLAGS) $(FREETYPE_CFLAGS)

noinst_PROGRAMS = test

test_SOURCES = main.c
test_LDADD = ../src/libharfbuzz-1.la $(FONTCONFIG_LIBS) $(FREETYPE_LIBS)
