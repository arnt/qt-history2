TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= ../../../../src/codecs/qeucjpcodec.h \
		  ../../../../src/codecs/qjiscodec.h \
		  ../../../../src/codecs/qsjiscodec.h \
		  ../../../../src/codecs/qjpunicode.h \
		  ../../../../src/codecs/qfontcodecs_p.h

SOURCES		= ../../../../src/codecs/qeucjpcodec.cpp \
		  ../../../../src/codecs/qjiscodec.cpp \
		  ../../../../src/codecs/qsjiscodec.cpp \
		  ../../../../src/codecs/qjpunicode.cpp \
		  ../../../../src/codecs/qfontjpcodec.cpp \
		  main.cpp

TARGET		= qjpcodecs
DESTDIR		= ../../../codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

