TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
QTDIR_build:REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/qgb18030codec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qgb18030codec.cpp \
		  ../../../../src/codecs/qfontcncodec.cpp \
		  main.cpp

TARGET		= qcncodecs
DESTDIR		= ../../../codecs


target.path += $$plugins.path/codecs
INSTALLS += target
