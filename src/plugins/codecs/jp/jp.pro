TARGET	 = qjpcodecs
include(../../qpluginbase.pri)

CONFIG	+= warn_on
DESTDIR	 = $$QT_BUILD_TREE/plugins/codecs

REQUIRES   = shared

HEADERS		= qjpunicode.h \
                  qeucjpcodec.h \
		  qjiscodec.h \
		  qsjiscodec.h 

SOURCES		= qeucjpcodec.cpp \
		  qjiscodec.cpp \
		  qsjiscodec.cpp \
		  qjpunicode.cpp \
		  main.cpp

unix:X11 {
	HEADERS += qfontjpcodec.h
	SOURCES += qfontjpcodec.cpp
}

target.path += $$plugins.path/codecs
INSTALLS += target
