TEMPLATE        = lib
CONFIG          += qt warn_on plugin
QTDIR_build:REQUIRES	= !bigcodecs

HEADERS         = ../../../../include/qbig5codec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES         = ../../../../src/codecs/qbig5codec.cpp \
		  ../../../../src/codecs/qfonttwcodec.cpp \
		  main.cpp

TARGET          = qtwcodecs
DESTDIR         = ../../../codecs


target.path += $$plugins.path/codecs
INSTALLS += target

