TEMPLATE = lib
TARGET  += qpng

CONFIG += qt plugin
DESTDIR = $$QT_BUILD_TREE/plugins/imageformats
VERSION = 1.0.0

QTDIR_build:REQUIRES = !no-png !png

SOURCES += main.cpp

system-png {
        unix:LIBS  += -lpng
        win32:LIBS += libpng.lib
}
!system-png {
	INCLUDEPATH += ../../../3rdparty/libpng
	INCLUDEPATH += ../../../3rdparty/zlib
	SOURCES += \
    	    ../../../3rdparty/libpng/png.c \
	    ../../../3rdparty/libpng/pngerror.c \
	    ../../../3rdparty/libpng/pngget.c \
	    ../../../3rdparty/libpng/pngmem.c \
	    ../../../3rdparty/libpng/pngpread.c \
	    ../../../3rdparty/libpng/pngread.c \
	    ../../../3rdparty/libpng/pngrio.c \
	    ../../../3rdparty/libpng/pngrtran.c \
	    ../../../3rdparty/libpng/pngrutil.c \
	    ../../../3rdparty/libpng/pngset.c \
	    ../../../3rdparty/libpng/pngtrans.c \
	    ../../../3rdparty/libpng/pngwio.c \
	    ../../../3rdparty/libpng/pngwrite.c \
	    ../../../3rdparty/libpng/pngwtran.c \
	    ../../../3rdparty/libpng/pngwutil.c
}

!system-zlib:SOURCES += \
	../../../3rdparty/zlib/adler32.c \
	../../../3rdparty/zlib/compress.c \
	../../../3rdparty/zlib/crc32.c \
	../../../3rdparty/zlib/deflate.c \
	../../../3rdparty/zlib/gzio.c \
	../../../3rdparty/zlib/infblock.c \
	../../../3rdparty/zlib/infcodes.c \
	../../../3rdparty/zlib/inffast.c \
	../../../3rdparty/zlib/inflate.c \
	../../../3rdparty/zlib/inftrees.c \
	../../../3rdparty/zlib/infutil.c \
	../../../3rdparty/zlib/trees.c \
	../../../3rdparty/zlib/uncompr.c \
	../../../3rdparty/zlib/zutil.c

system-zlib:unix:LIBS += -lz
system-zlib:mac:LIBS += -lz


target.path += $$plugins.path/imageformats
INSTALLS += target
