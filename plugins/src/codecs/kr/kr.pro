TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../src/codecs/qeuckrcodec.h \
		  ../../../../src/codecs/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qeuckrcodec.cpp \
		  ../../../../src/codecs/qfontkrcodec.cpp \
		  main.cpp

TARGET		= qkrcodecs
DESTDIR		= ../../../codecs
DEFINES		+= QT_PLUGIN_CODECS_KR

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

