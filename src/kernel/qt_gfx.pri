# Qt graphics 

jpeg {
	unix:LIBS += -ljpeg
	win32:LIBS += $(QTDIR)/lib/libjpeg.lib
}
!jpeg:DEFINES += QT_NO_IMAGEIO_JPEG

png {
	PNG_INCLUDEPATH		= 3rdparty/libpng
	INCLUDEPATH        += $$PNG_INCLUDEPATH
	SOURCES	+= 3rdparty/libpng/png.c \
		  3rdparty/libpng/pngerror.c \
		  3rdparty/libpng/pngget.c \
		  3rdparty/libpng/pngmem.c \
		  3rdparty/libpng/pngpread.c \
		  3rdparty/libpng/pngread.c \
		  3rdparty/libpng/pngrio.c \
		  3rdparty/libpng/pngrtran.c \
		  3rdparty/libpng/pngrutil.c \
		  3rdparty/libpng/pngset.c \
		  3rdparty/libpng/pngtrans.c \
		  3rdparty/libpng/pngwio.c \
		  3rdparty/libpng/pngwrite.c \
		  3rdparty/libpng/pngwtran.c \
		  3rdparty/libpng/pngwutil.c
}

zlib {
	ZLIB_INCLUDEPATH	= 3rdparty/zlib
	INCLUDEPATH       += $$ZLIB_INCLUDEPATH
	SOURCES	+= 3rdparty/zlib/adler32.c \
		  3rdparty/zlib/compress.c \
		  3rdparty/zlib/crc32.c \
		  3rdparty/zlib/deflate.c \
		  3rdparty/zlib/gzio.c \
		  3rdparty/zlib/infblock.c \
		  3rdparty/zlib/infcodes.c \
		  3rdparty/zlib/inffast.c \
		  3rdparty/zlib/inflate.c \
		  3rdparty/zlib/inftrees.c \
		  3rdparty/zlib/infutil.c \
		  3rdparty/zlib/trees.c \
		  3rdparty/zlib/uncompr.c \
		  3rdparty/zlib/zutil.c
}

mng {
	unix:LIBS	+= -lmng -ljpeg
	MNG_INCLUDEPATH		= 3rdparty/libmng
#	INCLUDEPATH        += $$MNG_INCLUDEPATH
}
!mng:DEFINES += QT_NO_IMAGEIO_MNG

gif {
	# use Qt gif
	DEFINES += QT_BUILTIN_GIF_READER
}
