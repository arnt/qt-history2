TEMPLATE = lib
TARGET   = qtwcodecs

CONFIG  += qt warn_on plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/codecs

QTDIR_build:REQUIRES = "!contains(QT_CONFIG, bigcodecs)"

HEADERS  = ../../../../include/qbig5codec.h \
	   ../../../../include/private/qfontcodecs_p.h
SOURCES  = ../../../../src/codecs/qbig5codec.cpp \
	   ../../../../src/codecs/qfonttwcodec.cpp \
	   main.cpp


target.path += $$plugins.path/codecs
INSTALLS += target

