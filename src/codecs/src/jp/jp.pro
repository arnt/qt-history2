TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../qeucjpcodec.h \
		  ../../qjiscodec.h \
		  ../../qsjiscodec.h \
		  ../../qjpunicode.h \
		  ../../qfontcodecs_p.h

SOURCES		= ../../qeucjpcodec.cpp \
		  ../../qjiscodec.cpp \
		  ../../qsjiscodec.cpp \
		  ../../qjpunicode.cpp \
		  ../../qfontjpcodec.cpp \
		  main.cpp

TARGET		= qjpcodecs
DESTDIR		= ../../../../plugins/codecs
DEFINES		+= QT_PLUGIN_CODECS_JP

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

