TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../src/codecs/qgbkcodec.h \
		  ../../../../src/codecs/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qgbkcodec.cpp \
		  ../../../../src/codecs/qfontcncodec.cpp \
		  main.cpp

TARGET		= qcncodecs
DESTDIR		= ../../../codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target
