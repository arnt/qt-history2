TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/qeuckrcodec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qeuckrcodec.cpp \
		  ../../../../src/codecs/qfontkrcodec.cpp \
		  main.cpp

TARGET		= qkrcodecs
DESTDIR		= ../../../codecs

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/codecs
INSTALLS += target

