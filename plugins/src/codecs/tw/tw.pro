TEMPLATE        = lib
CONFIG          += qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS         = ../../../../include/qbig5codec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES         = ../../../../src/codecs/qbig5codec.cpp \
		  ../../../../src/codecs/qfonttwcodec.cpp \
		  main.cpp

TARGET          = qtwcodecs
DESTDIR         = ../../../codecs

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/codecs
INSTALLS += target

