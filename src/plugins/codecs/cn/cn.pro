TARGET	 = qcncodecs
include(../../qpluginbase.pri)

CONFIG	+= warn_on
DESTDIR	 = $$QT_BUILD_TREE/plugins/codecs

REQUIRES   = shared

HEADERS		= qgb18030codec.h

SOURCES		= qgb18030codec.cpp \
		  main.cpp

target.path += $$plugins.path/codecs
INSTALLS += target
