TEMPLATE = lib
TARGET  += qjpeg

CONFIG  += qt plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/imageformats

VERSION = 1.0.0
QTDIR_build:REQUIRES = !no-jpeg !jpeg

SOURCES += main.cpp
system-jpeg {
        unix:LIBS += -ljpeg
        win32:LIBS += libjpeg.lib
}
!system-jpeg {
	INCLUDEPATH += ../../../3rdparty/libjpeg
	SOURCES  += \
	    ../../../3rdparty/libjpeg/jcapimin.c \
	    ../../../3rdparty/libjpeg/jcapistd.c \
	    ../../../3rdparty/libjpeg/jccoefct.c \
	    ../../../3rdparty/libjpeg/jccolor.c \
	    ../../../3rdparty/libjpeg/jcdctmgr.c \
	    ../../../3rdparty/libjpeg/jchuff.c \
	    ../../../3rdparty/libjpeg/jcinit.c \
	    ../../../3rdparty/libjpeg/jcmainct.c \
	    ../../../3rdparty/libjpeg/jcmarker.c \
	    ../../../3rdparty/libjpeg/jcmaster.c \
	    ../../../3rdparty/libjpeg/jcomapi.c \
	    ../../../3rdparty/libjpeg/jcparam.c \
	    ../../../3rdparty/libjpeg/jcphuff.c \
	    ../../../3rdparty/libjpeg/jcprepct.c \
	    ../../../3rdparty/libjpeg/jcsample.c \
	    ../../../3rdparty/libjpeg/jctrans.c \
	    ../../../3rdparty/libjpeg/jdapimin.c \
	    ../../../3rdparty/libjpeg/jdapistd.c \
	    ../../../3rdparty/libjpeg/jdatadst.c \
	    ../../../3rdparty/libjpeg/jdatasrc.c \
	    ../../../3rdparty/libjpeg/jdcoefct.c \
	    ../../../3rdparty/libjpeg/jdcolor.c \
	    ../../../3rdparty/libjpeg/jddctmgr.c \
	    ../../../3rdparty/libjpeg/jdhuff.c \
	    ../../../3rdparty/libjpeg/jdinput.c \
	    ../../../3rdparty/libjpeg/jdmainct.c \
	    ../../../3rdparty/libjpeg/jdmarker.c \
	    ../../../3rdparty/libjpeg/jdmaster.c \
	    ../../../3rdparty/libjpeg/jdmerge.c \
	    ../../../3rdparty/libjpeg/jdphuff.c \
	    ../../../3rdparty/libjpeg/jdpostct.c \
	    ../../../3rdparty/libjpeg/jdsample.c \
	    ../../../3rdparty/libjpeg/jdtrans.c \
	    ../../../3rdparty/libjpeg/jerror.c \
	    ../../../3rdparty/libjpeg/jfdctflt.c \
	    ../../../3rdparty/libjpeg/jfdctfst.c \
	    ../../../3rdparty/libjpeg/jfdctint.c \
	    ../../../3rdparty/libjpeg/jidctflt.c \
	    ../../../3rdparty/libjpeg/jidctfst.c \
	    ../../../3rdparty/libjpeg/jidctint.c \
	    ../../../3rdparty/libjpeg/jidctred.c \
	    ../../../3rdparty/libjpeg/jmemmgr.c \
	    ../../../3rdparty/libjpeg/jquant1.c \
	    ../../../3rdparty/libjpeg/jquant2.c \
	    ../../../3rdparty/libjpeg/jutils.c \
	    ../../../3rdparty/libjpeg/jmemnobs.c
}


target.path += $$plugins.path/imageformats
INSTALLS += target
