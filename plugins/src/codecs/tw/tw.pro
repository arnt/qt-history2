TEMPLATE        = lib
CONFIG          += qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS         = ../../../../src/codecs/qbig5codec.h \
		  ../../../../src/codecs/qfontcodecs_p.h

SOURCES         = ../../../../src/codecs/qbig5codec.cpp \
		  ../../../../src/codecs/qfonttwcodec.cpp \
		  main.cpp

TARGET          = qtwcodecs
DESTDIR         = ../../../codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

