# Qt/Embedded 

embedded {
	DEFINES += QWS
	CONFIG -= opengl
	CONFIG	+= png zlib
	CONFIG -= jpeg
	CONFIG -= x11
	LIBS -= -dl
	SUBLIBS += freetype png mng z
	jpeg:SUBLIBS += jpeg

	HEADERS += $$STYLES_H/qcompactstyle.h \
		  $$KERNEL_H/qfontmanager_qws.h \
		  $$KERNEL_H/qfontfactorybdf_qws.h \
		  $$KERNEL_H/qfontfactoryttf_qws.h \
		  $$KERNEL_H/qmemorymanager_qws.h \
		  $$KERNEL_H/qwsmanager_qws.h \
		  $$KERNEL_H/qgfx_qws.h \
		  $$KERNEL_H/qgfxraster_qws.h \
		  $$KERNEL_H/qgfxlinuxfb_qws.h \
		  $$KERNEL_H/qgfxvnc_qws.h \
		  $$KERNEL_H/qwindowsystem_qws.h \
		  $$KERNEL_H/qwscursor_qws.h \
		  $$KERNEL_H/qwsmouse_qws.h \
		  $$KERNEL_H/qlock_qws.h \
		  $$KERNEL_H/qsoundqss_qws.h \
		  $$KERNEL_H/qwsregionmanager_qws.h \
		  $$KERNEL_H/qwsdisplay_qws.h \
		  $$KERNEL_H/qwssocket_qws.h 

         SOURCES +=  $$KERNEL_CPP/qapplication_qws.cpp \
		  $$KERNEL_CPP/qclipboard_qws.cpp \
		  $$KERNEL_CPP/qcolor_qws.cpp \
		  $$KERNEL_CPP/qcursor_qws.cpp \
		  $$KERNEL_CPP/qdnd_qws.cpp \
		  $$KERNEL_CPP/qfont_qws.cpp \
		  $$KERNEL_CPP/qpixmap_qws.cpp \
		  $$KERNEL_CPP/qprinter_qws.cpp \
		  $$KERNEL_CPP/qpaintdevice_qws.cpp \
		  $$KERNEL_CPP/qpainter_qws.cpp \
		  $$KERNEL_CPP/qregion_qws.cpp \
		  $$KERNEL_CPP/qsoundqss_qws.cpp \
		  $$KERNEL_CPP/qsound_qws.cpp \
		  $$KERNEL_CPP/qwidget_qws.cpp \
		  $$KERNEL_CPP/qgfx_qws.cpp \
		  $$KERNEL_CPP/qgfxraster_qws.cpp \
		  $$KERNEL_CPP/qfontmanager_qws.cpp \
		  $$KERNEL_CPP/qfontfactorybdf_qws.cpp \
		  $$KERNEL_CPP/qfontfactoryttf_qws.cpp \
		  $$KERNEL_CPP/qmemorymanager_qws.cpp \
		  $$KERNEL_CPP/qwscommand_qws.cpp \
		  $$KERNEL_CPP/qwsevent_qws.cpp \
		  $$KERNEL_CPP/qwindowsystem_qws.cpp \
		  $$KERNEL_CPP/qkeyboard_qws.cpp \
		  $$KERNEL_CPP/qwscursor_qws.cpp \
		  $$KERNEL_CPP/qwsmouse_qws.cpp \
		  $$KERNEL_CPP/qwsmanager_qws.cpp \
		  $$KERNEL_CPP/qwsproperty_qws.cpp \
		  $$KERNEL_CPP/qlock_qws.cpp \
		  $$KERNEL_CPP/qwsregionmanager_qws.cpp \
		  $$KERNEL_CPP/qwssocket_qws.cpp

	ft:SOURCES+=3rdparty/freetype2/config/unix/ftsystem.c \
                3rdparty/freetype2/src/base/ftdebug.c \
                3rdparty/freetype2/src/base/ftinit.c \
                3rdparty/freetype2/src/base/ftbase.c \
                3rdparty/freetype2/src/base/ftbbox.c \
                3rdparty/freetype2/src/base/ftraster.c \
                3rdparty/freetype2/src/base/ftoutln.c \
                3rdparty/freetype2/src/psnames/psdriver.c \
                3rdparty/freetype2/src/sfnt/sfnt.c \
                3rdparty/freetype2/src/truetype/truetype.c \
                3rdparty/freetype2/src/type1/type1.c

	# Rather ridiculous. Perhaps we should ships symlinks and
	# only have one here.
	ft:INCLUDEPATH += 3rdparty/freetype2/src/shared \
		3rdparty/freetype2/src/sfnt \
		3rdparty/freetype2/include \
		3rdparty/freetype2/src/base \
		3rdparty/freetype2/config/unix \
		3rdparty/freetype2/config \
		3rdparty/freetype2/src/truetype \
		3rdparty/freetype2/src/type1 \
		3rdparty/freetype2/src/psnames

	PRECOMPH=$(QTDIR)/src/kernel/qt.h
	INCLUDEPATH += 3rdparty/freetype2/include 3rdparty/libpng 3rdparty/zlib
	MAKELIBz = $(MAKE) -C 3rdparty/zlib -f Makefile$$DASHCROSS; \
			cp 3rdparty/zlib/libz.a tmp
	MAKELIBfreetype = $(MAKE) -C 3rdparty/freetype2 CONFIG_MK=config$$DASHCROSS.mk OBJ_DIR=../../tmp \
			    ../../tmp/libfreetype.a
	MAKELIBpng = $(MAKE) -C 3rdparty/libpng \
			    -f scripts/makefile.linux$$DASHCROSS; \
			    cp 3rdparty/libpng/libpng.a tmp
	MAKELIBmng = $(MAKE) -C 3rdparty/libmng \
			    -f makefiles/makefile.linux$$DASHCROSS; \
			    cp 3rdparty/libmng/libmng.a tmp
	MAKELIBjpeg = $(MAKE) -C 3rdparty/jpeglib -f makefile.unix$$DASHCROSS; \
			    cp 3rdparty/jpeglib/libjpeg.a tmp
}

