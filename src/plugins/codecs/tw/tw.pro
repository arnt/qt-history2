TEMPLATE = lib
TARGET   = qtwcodecs

CONFIG  += qt warn_on plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/codecs

QTDIR_build:REQUIRES = "!contains(QT_CONFIG, bigcodecs)"
REQUIRES   = shared

HEADERS  = ../../../../include/QtCore/private/qbig5codec_p.h \
	   ../../../../include/QtCore/private/qfontcodecs_p.h

SOURCES  = ../../../core/codecs/qbig5codec.cpp \
	   ../../../core/codecs/qfonttwcodec.cpp \
	   main.cpp

target.path += $$plugins.path/codecs
INSTALLS += target
