TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../include/qgbkcodec.h \
		  ../../../../include/private/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qgbkcodec.cpp \
		  ../../../../src/codecs/qfontcncodec.cpp \
		  main.cpp

TARGET		= qcncodecs
DESTDIR		= ../../../codecs

isEmpty(plugins.path):plugins.path=$$QT_PREFIX/plugins
target.path += $$plugins.path/codecs
INSTALLS += target
