TEMPLATE = lib
TARGET  += qmng

CONFIG  += qt plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/imageformats

VERSION = 1.0.0
QTDIR_build:REQUIRES = !no-mng !mng

SOURCES += main.cpp

win32-borland {
	QMAKE_CFLAGS_WARN_ON	+= -w-par
	QMAKE_CXXFLAGS_WARN_ON	+= -w-par
}
win32: CONFIG-=zlib system-zlib jpeg system-jpeg

system-mng {
        win32:LIBS += libmng.lib
        unix:LIBS  += -lmng
}
!system-mng {
	INCLUDEPATH += ../../../3rdparty/libmng
	SOURCES += \
	../../../3rdparty/libmng/libmng_callback_xs.c \
	../../../3rdparty/libmng/libmng_chunk_io.c \
	../../../3rdparty/libmng/libmng_chunk_prc.c \
	../../../3rdparty/libmng/libmng_chunk_xs.c \
	../../../3rdparty/libmng/libmng_cms.c \
	../../../3rdparty/libmng/libmng_display.c \
	../../../3rdparty/libmng/libmng_dither.c \
	../../../3rdparty/libmng/libmng_error.c \
	../../../3rdparty/libmng/libmng_filter.c \
	../../../3rdparty/libmng/libmng_hlapi.c \
	../../../3rdparty/libmng/libmng_jpeg.c \
	../../../3rdparty/libmng/libmng_object_prc.c \
	../../../3rdparty/libmng/libmng_pixels.c \
	../../../3rdparty/libmng/libmng_prop_xs.c \
	../../../3rdparty/libmng/libmng_read.c \
	../../../3rdparty/libmng/libmng_trace.c \
	../../../3rdparty/libmng/libmng_write.c \
	../../../3rdparty/libmng/libmng_zlib.c

	HEADERS += ../../../3rdparty/libmng/libmng.h \
	../../../3rdparty/libmng/libmng_chunks.h \
	../../../3rdparty/libmng/libmng_chunk_io.h \
	../../../3rdparty/libmng/libmng_chunk_prc.h \
	../../../3rdparty/libmng/libmng_cms.h \
	../../../3rdparty/libmng/libmng_conf.h \
	../../../3rdparty/libmng/libmng_data.h \
	../../../3rdparty/libmng/libmng_display.h \
	../../../3rdparty/libmng/libmng_dither.h \
	../../../3rdparty/libmng/libmng_error.h \
	../../../3rdparty/libmng/libmng_filter.h \
	../../../3rdparty/libmng/libmng_jpeg.h \
	../../../3rdparty/libmng/libmng_memory.h \
	../../../3rdparty/libmng/libmng_objects.h \
	../../../3rdparty/libmng/libmng_object_prc.h \
	../../../3rdparty/libmng/libmng_pixels.h \
	../../../3rdparty/libmng/libmng_read.h \
	../../../3rdparty/libmng/libmng_trace.h \
	../../../3rdparty/libmng/libmng_types.h \
	../../../3rdparty/libmng/libmng_write.h \
	../../../3rdparty/libmng/libmng_zlib.h
}

!system-zlib {
	INCLUDEPATH += ../../../3rdparty/zlib
	SOURCES+= \
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
}
!no-zlib:!zlib:unix:LIBS += -lz
!no-zlib:!zlib:mac:LIBS += -lz

system-jpeg {
        unix:LIBS  += -ljpeg
        win32:LIBS += libjpeg.lib
}
!system-jpeg {
	INCLUDEPATH += ../../../3rdparty/libjpeg
	SOURCES += \
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
