# Qt image handling 

# Qt kernel module

HEADERS += \
	image/qasyncimageio.h \
	image/qbitmap.h \
	image/qgif.h \
	image/qiconset.h \
	image/qimage.h \
	image/qimageformatplugin.h \
	image/qimageformatinterface_p.h \
	image/qmovie.h \
	image/qpicture.h \
	image/qpictureformatinterface_p.h \
	image/qpictureformatplugin.h \
	image/qpixmap.h \
	image/qpixmapcache.h


SOURCES += \
	image/qasyncimageio.cpp \
	image/qbitmap.cpp \
	image/qiconset.cpp \
	image/qimage.cpp \
	image/qimageformatplugin.cpp \
	image/qimage_gui.cpp \
	image/qmovie.cpp \
	image/qpicture.cpp \
	image/qpictureformatplugin.cpp \
	image/qpixmap.cpp \
	image/qpixmapcache.cpp


win32:SOURCES += image/qpixmap_win.cpp
unix:x11:SOURCES += image/qpixmap_x11.cpp
!embedded:!x11:mac:SOURCES += image/qpixmap_mac.cpp
embedded:SOURCES += image/qpixmap_qws.cpp


#mng support
HEADERS += image/qmngio.h
SOURCES += image/qmngio.cpp
mng {
        system-mng {
	    win32:LIBS += libmng.lib
            unix:LIBS  += -lmng
        } else {
	   INCLUDEPATH        += ../3rdparty/libmng
	   SOURCES += ../3rdparty/libmng/libmng_callback_xs.c \
		  ../3rdparty/libmng/libmng_chunk_io.c \
		  ../3rdparty/libmng/libmng_chunk_prc.c \
		  ../3rdparty/libmng/libmng_chunk_xs.c \
		  ../3rdparty/libmng/libmng_cms.c \
		  ../3rdparty/libmng/libmng_display.c \
		  ../3rdparty/libmng/libmng_dither.c \
		  ../3rdparty/libmng/libmng_error.c \
		  ../3rdparty/libmng/libmng_filter.c \
		  ../3rdparty/libmng/libmng_hlapi.c \
		  ../3rdparty/libmng/libmng_jpeg.c \
		  ../3rdparty/libmng/libmng_object_prc.c \
		  ../3rdparty/libmng/libmng_pixels.c \
		  ../3rdparty/libmng/libmng_prop_xs.c \
		  ../3rdparty/libmng/libmng_read.c \
		  ../3rdparty/libmng/libmng_trace.c \
		  ../3rdparty/libmng/libmng_write.c \
		  ../3rdparty/libmng/libmng_zlib.c
        }
	no-jpeg {
		message(Use of mng requires support for jpeg)
		CONFIG += jpeg
	} else:!jpeg {
		message(Use of mng requires support for jpeg)
		CONFIG += jpeg
	}
}
else:DEFINES += QT_NO_IMAGEIO_MNG

#jpeg support..
HEADERS += image/qjpegio.h 
SOURCES += image/qjpegio.cpp
jpeg {
        system-jpeg {
	   unix:LIBS += -ljpeg
	   win32:LIBS += libjpeg.lib
	} else {
	    INCLUDEPATH += ../3rdparty/libjpeg
	    SOURCES += ../3rdparty/libjpeg/jcapimin.c \
		  ../3rdparty/libjpeg/jcapistd.c \
		  ../3rdparty/libjpeg/jccoefct.c \
		  ../3rdparty/libjpeg/jccolor.c \
		  ../3rdparty/libjpeg/jcdctmgr.c \
		  ../3rdparty/libjpeg/jchuff.c \
		  ../3rdparty/libjpeg/jcinit.c \
		  ../3rdparty/libjpeg/jcmainct.c \
		  ../3rdparty/libjpeg/jcmarker.c \
		  ../3rdparty/libjpeg/jcmaster.c \
		  ../3rdparty/libjpeg/jcomapi.c \
		  ../3rdparty/libjpeg/jcparam.c \
		  ../3rdparty/libjpeg/jcphuff.c \
		  ../3rdparty/libjpeg/jcprepct.c \
		  ../3rdparty/libjpeg/jcsample.c \
		  ../3rdparty/libjpeg/jctrans.c \
		  ../3rdparty/libjpeg/jdapimin.c \
		  ../3rdparty/libjpeg/jdapistd.c \
		  ../3rdparty/libjpeg/jdatadst.c \
		  ../3rdparty/libjpeg/jdatasrc.c \
		  ../3rdparty/libjpeg/jdcoefct.c \
		  ../3rdparty/libjpeg/jdcolor.c \
		  ../3rdparty/libjpeg/jddctmgr.c \
		  ../3rdparty/libjpeg/jdhuff.c \
		  ../3rdparty/libjpeg/jdinput.c \
		  ../3rdparty/libjpeg/jdmainct.c \
		  ../3rdparty/libjpeg/jdmarker.c \
		  ../3rdparty/libjpeg/jdmaster.c \
		  ../3rdparty/libjpeg/jdmerge.c \
		  ../3rdparty/libjpeg/jdphuff.c \
		  ../3rdparty/libjpeg/jdpostct.c \
		  ../3rdparty/libjpeg/jdsample.c \
		  ../3rdparty/libjpeg/jdtrans.c \
		  ../3rdparty/libjpeg/jerror.c \
		  ../3rdparty/libjpeg/jfdctflt.c \
		  ../3rdparty/libjpeg/jfdctfst.c \
		  ../3rdparty/libjpeg/jfdctint.c \
		  ../3rdparty/libjpeg/jidctflt.c \
		  ../3rdparty/libjpeg/jidctfst.c \
		  ../3rdparty/libjpeg/jidctint.c \
		  ../3rdparty/libjpeg/jidctred.c \
		  ../3rdparty/libjpeg/jmemmgr.c \
		  ../3rdparty/libjpeg/jquant1.c \
		  ../3rdparty/libjpeg/jquant2.c \
		  ../3rdparty/libjpeg/jutils.c \
		  ../3rdparty/libjpeg/jmemnobs.c
        }
}
else:DEFINES += QT_NO_IMAGEIO_JPEG

#png support
HEADERS += image/qpngio.h
SOURCES += image/qpngio.cpp
png {
        system-png {
	    unix:LIBS  += -lpng
	    win32:LIBS += libpng.lib
	} else {
	    INCLUDEPATH  += ../3rdparty/libpng ../3rdparty/zlib
	    SOURCES	+= ../3rdparty/libpng/png.c \
		  ../3rdparty/libpng/pngerror.c \
		  ../3rdparty/libpng/pngget.c \
		  ../3rdparty/libpng/pngmem.c \
		  ../3rdparty/libpng/pngpread.c \
		  ../3rdparty/libpng/pngread.c \
		  ../3rdparty/libpng/pngrio.c \
		  ../3rdparty/libpng/pngrtran.c \
		  ../3rdparty/libpng/pngrutil.c \
		  ../3rdparty/libpng/pngset.c \
		  ../3rdparty/libpng/pngtrans.c \
		  ../3rdparty/libpng/pngwio.c \
		  ../3rdparty/libpng/pngwrite.c \
		  ../3rdparty/libpng/pngwtran.c \
		  ../3rdparty/libpng/pngwutil.c 
        }
}
else:DEFINES += QT_NO_IMAGEIO_PNG

#use Qt gif
gif:DEFINES += QT_BUILTIN_GIF_READER=1

