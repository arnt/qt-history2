TEMPLATE = lib
TARGET	 = qjpcodecs

CONFIG	+= qt warn_on plugin
DESTDIR	 = $$QT_BUILD_TREE/plugins/codecs

QTDIR_build:REQUIRES	= "!contains(QT_CONFIG, bigcodecs)"
REQUIRES   = shared

HEADERS		= ../../../../include/QtCore/qjpunicode.h \
                  ../../../../include/QtCore/private/qeucjpcodec_p.h \
		  ../../../../include/QtCore/private/qjiscodec_p.h \
		  ../../../../include/QtCore/private/qsjiscodec_p.h \
		  ../../../../include/QtCore/private/qfontcodecs_p.h

SOURCES		= ../../../core/codecs/qeucjpcodec.cpp \
		  ../../../core/codecs/qjiscodec.cpp \
		  ../../../core/codecs/qsjiscodec.cpp \
		  ../../../core/codecs/qjpunicode.cpp \
		  ../../../core/codecs/qfontjpcodec.cpp \
		  main.cpp

target.path += $$plugins.path/codecs
INSTALLS += target
