TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/qeucjpcodec.h \
		  ../../../../include/qjiscodec.h \
		  ../../../../include/qsjiscodec.h \
		  ../../../../include/qjpunicode.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qeucjpcodec.cpp \
		  ../../../../src/codecs/qjiscodec.cpp \
		  ../../../../src/codecs/qsjiscodec.cpp \
		  ../../../../src/codecs/qjpunicode.cpp \
		  ../../../../src/codecs/qfontjpcodec.cpp \
		  main.cpp

TARGET		= qjpcodecs
DESTDIR		= ../../../codecs


target.path += $$plugins.path/codecs
INSTALLS += target

