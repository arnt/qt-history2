TARGET   = qtwcodecs
include(../../qpluginbase.pri)

CONFIG  += warn_on
DESTDIR  = $$QT_BUILD_TREE/plugins/codecs

REQUIRES   = shared

HEADERS  = qbig5codec.h

SOURCES  = qbig5codec.cpp \
	   main.cpp

target.path += $$plugins.path/codecs
INSTALLS += target
