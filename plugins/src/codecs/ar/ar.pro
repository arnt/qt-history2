TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qfontarcodec.cpp \
		  main.cpp

TARGET		= qarcodecs
DESTDIR		= ../../../codecs


target.path += $$plugins.path/codecs
INSTALLS += target
