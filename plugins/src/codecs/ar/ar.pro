TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../src/codecs/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qfontarcodec.cpp \
		  main.cpp

TARGET		= qarcodecs
DESTDIR		= ../../../codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target
