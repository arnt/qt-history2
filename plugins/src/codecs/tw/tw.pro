TEMPLATE = lib
TARGET   = qtwcodecs

CONFIG  += qt warn_on plugin
DESTDIR  = ../../../codecs

QTDIR_build:REQUIRES = !bigcodecs

HEADERS  = ../../../../include/qbig5codec.h \
	   ../../../../include/private/qfontcodecs_p.h
SOURCES  = ../../../../src/codecs/qbig5codec.cpp \
	   ../../../../src/codecs/qfonttwcodec.cpp \
	   main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

