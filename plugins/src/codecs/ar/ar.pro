TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qfontarcodec.cpp \
		  main.cpp

TARGET		= qarcodecs
DESTDIR		= ../../../codecs

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/codecs
INSTALLS += target
