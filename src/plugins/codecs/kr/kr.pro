TARGET	 = qkrcodecs
include(../../qpluginbase.pri)

CONFIG	+= warn_on
DESTDIR	 = $$QT_BUILD_TREE/plugins/codecs

QTDIR_build:REQUIRES = "!contains(QT_CONFIG, bigcodecs)"
REQUIRES   = shared

HEADERS		= ../../../../include/QtCore/private/qeuckrcodec_p.h \
		  ../../../../include/QtCore/private/qfontcodecs_p.h

SOURCES		= ../../../core/codecs/qeuckrcodec.cpp \
		  ../../../core/codecs/qfontkrcodec.cpp \
		  main.cpp

target.path += $$plugins.path/codecs
INSTALLS += target
