# Qt graphics 

#jpeg support..
jpeg {
	unix:LIBS += -ljpeg
	win32:LIBS += $(QTDIR)/lib/libjpeg.lib
	HEADERS += $$KERNEL_H/qjpegio.h 
	SOURCES += $$KERNEL_CPP/qjpegio.cpp
}
!jpeg:DEFINES += QT_NO_IMAGEIO_JPEG


#png support
HEADERS+=$$KERNEL_H/qpngio.h
SOURCES+=$$KERNEL_CPP/qpngio.cpp
png {
	INCLUDEPATH  += 3rdparty/libpng
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
!png:LIBS += -lpng

#zlib support
zlib {
	INCLUDEPATH       += 3rdparty/zlib
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
!zlib:LIBS += -lz

#mng support
mng {
	REQUIRES = jpeg
	!jpeg:message(mng support requires jpeg)

	unix:LIBS	+= -lmng
	INCLUDEPATH        += 3rdparty/libmng
	HEADERS += $$KERNEL_H/qmngio.h
	SOURCES += $$KERNEL_CPP/qmngio.cpp
}
!mng:DEFINES += QT_NO_IMAGEIO_MNG

#use Qt gif
gif:DEFINES += QT_BUILTIN_GIF_READER

