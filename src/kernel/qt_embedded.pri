# Qt/Embedded 

embedded {
	ps2 {
		GSOS_INCLUDEPATH        = 3rdparty/gsos
		INCLUDEPATH         += $$GSOS_INCLUDEPATH
		QWSSUBLIBS += gsos
		MAKELIBgsos = $(MAKE) -C 3rdparty/gsos; \
			cp 3rdparty/gsos/libgsos.a tmp
	}
	!ps2:DEFINES += QT_NO_QWS_PS2

	CONFIG -= opengl
	CONFIG	+= png zlib
	CONFIG -= jpeg
	CONFIG -= x11
	LIBS -= -dl
	QWSSUBLIBS = freetype png jpeg mng z
	SUBLIBS = $$QWSSUBLIBS
	HEADERS += $$KERNEL_H/qfontmanager_qws.h \
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
		  $$KERNEL_H/qwsregionmanager_qws.h \
		  $$KERNEL_H/qwsdisplay_qws.h \
		  $$KERNEL_H/qwssocket_qws.h 

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

