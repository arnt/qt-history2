TEMPLATE        = lib
CONFIG          += qt warn_on plugin
REQUIRES	= dll !bigcodecs

HEADERS         = ../../qbig5codec.h \
		  ../../qfontcodecs_p.h

SOURCES         = ../../qbig5codec.cpp \
		  ../../qfonttwcodec.cpp \
		  main.cpp

TARGET          = qtwcodecs
DESTDIR         = ../../../../plugins/codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

