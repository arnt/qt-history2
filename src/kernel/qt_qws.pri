# Qt/Embedded 

embedded {
	CONFIG -= opengl
	CONFIG -= jpeg
	CONFIG -= x11
	LIBS -= -dl
	KERNEL_P        = kernel

	HEADERS += $$STYLES_H/qcompactstyle.h \
		    $$KERNEL_H/qcopchannel_qws.h \
		    $$KERNEL_H/qdirectpainter_qws.h \
		    $$KERNEL_H/qfontfactorybdf_qws.h \
		    $$KERNEL_H/qfontfactoryttf_qws.h \
		    $$KERNEL_H/qfontmanager_qws.h \
		    $$KERNEL_H/qgfx_qws.h \
		    $$KERNEL_H/qgfxlinuxfb_qws.h \
		    $$KERNEL_H/qgfxmatroxdefs_qws.h \
		    $$KERNEL_H/qgfxraster_qws.h \
		    $$KERNEL_H/qgfxvoodoodefs_qws.h \
		    $$KERNEL_H/qlock_qws.h \
		    $$KERNEL_H/qmemorymanager_qws.h \
		    $$KERNEL_H/qsoundqss_qws.h \
		    $$KERNEL_H/qwindowsystem_qws.h \
		    $$KERNEL_H/qwsbeosdecoration_qws.h \
		    $$KERNEL_H/qwscommand_qws.h \
		    $$KERNEL_H/qwscursor_qws.h \
		    $$KERNEL_H/qwsdecoration_qws.h \
		    $$KERNEL_H/qwsdefaultdecoration_qws.h \
		    $$KERNEL_H/qwsdisplay_qws.h \
		    $$KERNEL_H/qwsevent_qws.h \
		    $$KERNEL_H/qwshydrodecoration_qws.h \
		    $$KERNEL_H/qwskde2decoration_qws.h \
		    $$KERNEL_H/qwskdedecoration_qws.h \
		    $$KERNEL_H/qwsmanager_qws.h \
		    $$KERNEL_H/qwsmouse_qws.h \
		    $$KERNEL_H/qwsproperty_qws.h \
		    $$KERNEL_H/qwsregionmanager_qws.h \
		    $$KERNEL_H/qwssocket_qws.h \
		    $$KERNEL_H/qwsutils_qws.h \
		    $$KERNEL_H/qwswindowsdecoration_qws.h \
		    $$KERNEL_P/qgfxdriverinterface_p.h \
		    $$KERNEL_H/qgfxdriverplugin_qws.h \
		    $$KERNEL_H/qgfxdriverfactory_qws.h

         SOURCES +=  $$KERNEL_CPP/qapplication_qws.cpp \
		    $$KERNEL_CPP/qclipboard_qws.cpp \
		    $$KERNEL_CPP/qcolor_qws.cpp \
		    $$KERNEL_CPP/qcopchannel_qws.cpp \
		    $$KERNEL_CPP/qcursor_qws.cpp \
		    $$KERNEL_CPP/qdesktopwidget_qws.cpp \
		    $$KERNEL_CPP/qdirectpainter_qws.cpp \
		    $$KERNEL_CPP/qdnd_qws.cpp \
		    $$KERNEL_CPP/qfont_qws.cpp \
		    $$KERNEL_CPP/qfontfactorybdf_qws.cpp \
		    $$KERNEL_CPP/qfontfactoryttf_qws.cpp \
		    $$KERNEL_CPP/qfontmanager_qws.cpp \
		    $$KERNEL_CPP/qgfx_qws.cpp \
		    $$KERNEL_CPP/qgfxraster_qws.cpp \
		    $$KERNEL_CPP/qgfxlinuxfb_qws.cpp \
		    $$KERNEL_CPP/qkeyboard_qws.cpp \
		    $$KERNEL_CPP/qlock_qws.cpp \
		    $$KERNEL_CPP/qmemorymanager_qws.cpp \
		    $$KERNEL_CPP/qpaintdevice_qws.cpp \
		    $$KERNEL_CPP/qpainter_qws.cpp \
		    $$KERNEL_CPP/qpixmap_qws.cpp \
		    $$KERNEL_CPP/qregion_qws.cpp \
		    $$KERNEL_CPP/qsound_qws.cpp \
		    $$KERNEL_CPP/qsoundqss_qws.cpp \
		    $$KERNEL_CPP/qwidget_qws.cpp \
		    $$KERNEL_CPP/qwindowsystem_qws.cpp \
		    $$KERNEL_CPP/qwsbeosdecoration_qws.cpp \
		    $$KERNEL_CPP/qwscommand_qws.cpp \
		    $$KERNEL_CPP/qwscursor_qws.cpp \
		    $$KERNEL_CPP/qwsdecoration_qws.cpp \
		    $$KERNEL_CPP/qwsdefaultdecoration_qws.cpp \
		    $$KERNEL_CPP/qwsevent_qws.cpp \
		    $$KERNEL_CPP/qwshydrodecoration_qws.cpp \
		    $$KERNEL_CPP/qwskde2decoration_qws.cpp \
		    $$KERNEL_CPP/qwskdedecoration_qws.cpp \
		    $$KERNEL_CPP/qwsmanager_qws.cpp \
		    $$KERNEL_CPP/qwsmouse_qws.cpp \
		    $$KERNEL_CPP/qwsproperty_qws.cpp \
		    $$KERNEL_CPP/qwsregionmanager_qws.cpp \
		    $$KERNEL_CPP/qwssocket_qws.cpp \
		    $$KERNEL_CPP/qwswindowsdecoration_qws.cpp \
		    $$KERNEL_CPP/qgfxdriverplugin_qws.cpp \
		    $$KERNEL_CPP/qgfxdriverfactory_qws.cpp

	ft:SOURCES += \
		3rdparty/freetype/builds/unix/ftsystem.c \
		3rdparty/freetype/src/base/ftdebug.c \
		3rdparty/freetype/src/base/ftinit.c \
		3rdparty/freetype/src/base/ftbase.c \
		3rdparty/freetype/src/base/ftglyph.c \
		3rdparty/freetype/src/base/ftmm.c \
		3rdparty/freetype/src/base/ftbbox.c \
		3rdparty/freetype/src/autohint/autohint.c \
		3rdparty/freetype/src/cache/ftcache.c \
		3rdparty/freetype/src/cff/cff.c \
		3rdparty/freetype/src/cid/type1cid.c \
		3rdparty/freetype/src/psaux/psaux.c \
		3rdparty/freetype/src/psnames/psmodule.c \
		3rdparty/freetype/src/raster/raster.c \
		3rdparty/freetype/src/sfnt/sfnt.c \
		3rdparty/freetype/src/smooth/smooth.c \
		3rdparty/freetype/src/truetype/truetype.c \
		3rdparty/freetype/src/type1/type1.c \
		3rdparty/freetype/src/winfonts/winfnt.c

	ft:INCLUDEPATH += \
		3rdparty/freetype/src \
		3rdparty/freetype/include \
		3rdparty/freetype/builds/unix

	else:DEFINES += QT_NO_FREETYPE

	contains( gfx-drivers, qvfb ) {
		HEADERS += $$KERNEL_H/qgfxvfb_qws.h
		SOURCES += $$KERNEL_CPP/qgfxvfb_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VFB

	contains( gfx-drivers, vnc ) {
		HEADERS += $$KERNEL_H/qgfxvnc_qws.h
		SOURCES += $$KERNEL_CPP/qgfxvnc_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VNC

	contains( gfx-drivers, transformed ) {
		HEADERS += $$KERNEL_H/qgfxtransformed_qws.h
		SOURCES += $$KERNEL_CPP/qgfxtransformed_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_TRANSFORMED

	contains( gfx-drivers, mach64 ) {
		HEADERS += $$KERNEL_H/qgfxmach64_qws.h \
			   $$KERNEL_H/qgfxmach64defs_qws.h
		SOURCES += $$KERNEL_CPP/qgfxmach64_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MACH64

	contains( gfx-drivers, shadow ) {
		HEADERS += $$KERNEL_H/qgfxshadow_qws.h
		SOURCES += $$KERNEL_CPP/qgfxshadow_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_SHADOWFB


	PRECOMPH=$(QTDIR)/include/qt.h
	INCLUDEPATH += 3rdparty/freetype2/include 3rdparty/libpng 3rdparty/zlib

	qnx { 
	SOURCES+= qwskeyboard_qnx.cpp \
		    qwsmouse_qnx.cpp
	}
}

