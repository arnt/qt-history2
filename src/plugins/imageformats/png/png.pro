TARGET  = qpng
include(../../qpluginbase.pri)

DESTDIR = $$QT_BUILD_TREE/plugins/imageformats

QTDIR_build:REQUIRES = "!contains(QT_CONFIG, no-png)" "!contains(QT_CONFIG, png)"

SOURCES += main.cpp

contains(QT_CONFIG, system-png) {
        unix:LIBS  += -lpng
        win32:LIBS += libpng.lib
}
!contains(QT_CONFIG, system-png) {
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

!contains(QT_CONFIG, system-zlib):SOURCES += \
	../../../3rdparty/zlib/adler32.c \
	../../../3rdparty/zlib/compress.c \
	../../../3rdparty/zlib/crc32.c \
	../../../3rdparty/zlib/deflate.c \
	../../../3rdparty/zlib/gzio.c \
	../../../3rdparty/zlib/inffast.c \
	../../../3rdparty/zlib/inflate.c \
	../../../3rdparty/zlib/inftrees.c \
	../../../3rdparty/zlib/trees.c \
	../../../3rdparty/zlib/uncompr.c \
	../../../3rdparty/zlib/zutil.c

contains(QT_CONFIG, system-zlib):unix:LIBS += -lz
contains(QT_CONFIG, system-zlib):mac:LIBS += -lz

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target
