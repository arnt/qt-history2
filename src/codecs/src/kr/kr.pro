TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= dll !bigcodecs

HEADERS		= ../../qeuckrcodec.h \
		  ../../qfontcodecs_p.h

SOURCES		= ../../qeuckrcodec.cpp \
		  ../../qfontkrcodec.cpp \
		  main.cpp

TARGET		= qkrcodecs
DESTDIR		= ../../../../plugins/codecs
DEFINES		+= QT_PLUGIN_CODECS_KR

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

