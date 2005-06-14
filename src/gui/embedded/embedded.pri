# Qt/Embedded

embedded {
	CONFIG -= opengl x11
	LIBS -= -dl
	KERNEL_P        = kernel

	!mac:HEADERS += embedded/qsoundqss_qws.h
	HEADERS += \
		    embedded/qcopchannel_qws.h \
		    embedded/qdecoration_qws.h \
		    embedded/qdecorationfactory_qws.h \
		    embedded/qdecorationplugin_qws.h \
		    embedded/qdirectpainter_qws.h \
		    embedded/qlock_p.h \
		    embedded/qscreen_qws.h \
		    embedded/qwindowsystem_qws.h \
		    embedded/qwscommand_qws.h \
		    embedded/qwscursor_qws.h \
		    embedded/qwsdisplay_qws.h \
		    embedded/qwsevent_qws.h \
		    embedded/qwsmanager_qws.h \
		    embedded/qwsmanager_p.h \
		    embedded/qwsproperty_qws.h \
		    embedded/qwssocket_qws.h \
		    embedded/qwsutils_qws.h

	 !mac:SOURCES += embedded/qsoundqss_qws.cpp
         SOURCES +=  \
		    embedded/qcopchannel_qws.cpp \
		    embedded/qdecoration_qws.cpp \
		    embedded/qdecorationfactory_qws.cpp \
		    embedded/qdecorationplugin_qws.cpp \
		    embedded/qdirectpainter_qws.cpp \
		    embedded/qlock.cpp \
		    embedded/qscreen_qws.cpp \
		    embedded/qwindowsystem_qws.cpp \
		    embedded/qwscommand_qws.cpp \
		    embedded/qwscursor_qws.cpp \
		    embedded/qwsevent_qws.cpp \
		    embedded/qwsmanager_qws.cpp \
		    embedded/qwsproperty_qws.cpp \
		    embedded/qwssocket_qws.cpp

	contains(QT_CONFIG, ft) {
	    SOURCES += \
		../3rdparty/freetype/builds/unix/ftsystem.c \
		../3rdparty/freetype/src/autohint/autohint.c \
		../3rdparty/freetype/src/base/ftbase.c \
		../3rdparty/freetype/src/base/ftbbox.c \
		../3rdparty/freetype/src/base/ftdebug.c \
		../3rdparty/freetype/src/base/ftglyph.c \
		../3rdparty/freetype/src/base/ftinit.c \
		../3rdparty/freetype/src/base/ftmm.c \
		../3rdparty/freetype/src/bdf/bdf.c \
		../3rdparty/freetype/src/cache/ftcache.c \
		../3rdparty/freetype/src/cff/cff.c \
		../3rdparty/freetype/src/cid/type1cid.c \
		../3rdparty/freetype/src/gzip/ftgzip.c \
		../3rdparty/freetype/src/pcf/pcf.c \
		../3rdparty/freetype/src/pfr/pfr.c \
		../3rdparty/freetype/src/psaux/psaux.c \
		../3rdparty/freetype/src/pshinter/pshinter.c \
		../3rdparty/freetype/src/psnames/psmodule.c \
		../3rdparty/freetype/src/raster/raster.c \
		../3rdparty/freetype/src/sfnt/sfnt.c \
		../3rdparty/freetype/src/smooth/smooth.c \
		../3rdparty/freetype/src/truetype/truetype.c \
		../3rdparty/freetype/src/type1/type1.c \
		../3rdparty/freetype/src/type42/type42.c \
		../3rdparty/freetype/src/winfonts/winfnt.c \
		../3rdparty/freetype/src/lzw/ftlzw.c

	    INCLUDEPATH += \
		../3rdparty/freetype/src \
		../3rdparty/freetype/include \
		../3rdparty/freetype/builds/unix

	    DEFINES += FT_CONFIG_OPTION_SYSTEM_ZLIB
	} else {
	    DEFINES += QT_NO_FREETYPE
	}

#
# Decorations
#
    contains( decorations, default ) {
    	HEADERS += embedded/qdecorationdefault_qws.h
    	SOURCES += embedded/qdecorationdefault_qws.cpp
    }
    contains( decorations, styled ) {
    	HEADERS += embedded/qdecorationstyled_qws.h
    	SOURCES += embedded/qdecorationstyled_qws.cpp
    }

    contains( decorations, windows ) {
    	HEADERS += embedded/qdecorationwindows_qws.h
    	SOURCES += embedded/qdecorationwindows_qws.cpp
    }

#
# Qt/Embedded Drivers
#
	HEADERS += embedded/qscreendriverplugin_qws.h \
		    embedded/qscreendriverfactory_qws.h \
		    embedded/qkbd_qws.h \
		    embedded/qkbddriverplugin_qws.h \
		    embedded/qkbddriverfactory_qws.h \
		    embedded/qmouse_qws.h \
		    embedded/qmousedriverplugin_qws.h \
		    embedded/qmousedriverfactory_qws.h

	SOURCES += embedded/qscreendriverplugin_qws.cpp \
		    embedded/qscreendriverfactory_qws.cpp \
		    embedded/qkbd_qws.cpp \
		    embedded/qkbddriverplugin_qws.cpp \
		    embedded/qkbddriverfactory_qws.cpp \
		    embedded/qmouse_qws.cpp \
		    embedded/qmousedriverplugin_qws.cpp \
		    embedded/qmousedriverfactory_qws.cpp

#
# Graphics drivers
#
        linux-* {
	        HEADERS += embedded/qscreenlinuxfb_qws.h
		SOURCES += embedded/qscreenlinuxfb_qws.cpp
	}

	contains( gfx-drivers, qvfb ) {
		HEADERS += embedded/qscreenvfb_qws.h
		SOURCES += embedded/qscreenvfb_qws.cpp
	}

	contains( gfx-drivers, vnc ) {
		HEADERS += embedded/qscreenvnc_qws.h
		SOURCES += embedded/qscreenvnc_qws.cpp
	}

#	!contains( DEFINES, QT_NO_QWS_LINUXFB):contains( gfx-drivers, vga16 ) {
#		HEADERS += embedded/qscreenvga16_qws.h
#		SOURCES += embedded/qscreenvga16_qws.cpp
#	}
#
#	contains( gfx-drivers, transformed ) {
#		HEADERS += embedded/qscreentransformed_qws.h
#		SOURCES += embedded/qscreentransformed_qws.cpp
#	}
#
#	contains( gfx-drivers, mach64 ) {
#		HEADERS += embedded/qscreenmach64_qws.h \
#			   embedded/qscreenmach64defs_qws.h
#		SOURCES += embedded/qscreenmach64_qws.cpp
#	}
#
#	contains( gfx-drivers, snap ) {
#		exists( $(SCITECH)/include/snap/graphics.h) {
#			HEADERS += embedded/qscreensnap_qws.h
#			SOURCES += embedded/qscreensnap_qws.cpp
#			INCLUDEPATH += $(SCITECH)/include
#			debug:LIBS	+= -L$(SCITECH)/lib/debug/linux/gcc/x86/so -lpm
#			else:LIBS	+= -L$(SCITECH)/lib/release/linux/gcc/x86/so -lpm
#		}
#		else {
#			message("SciTech SNAP SDK is not properly set up! Please make sure the SCITECH")
#			message("environment variable is pointing to the SciTech SNAP SDK.")
#			error("Please fix and re-build the makefiles.")
#		}
#	}
#
#	contains( gfx-drivers, voodoo ) {
#		HEADERS += embedded/qscreenvoodoo_qws.h \
#			   embedded/qscreenvoodoodefs_qws.h
#		SOURCES += embedded/qscreenvoodoo_qws.cpp
#	}
#
#	contains( gfx-drivers, matrox ) {
#		HEADERS += embedded/qscreenmatrox_qws.h \
#			   embedded/qscreenmatroxdefs_qws.h
#		SOURCES += embedded/qscreenmatrox_qws.cpp
#	}
#
#	contains( gfx-drivers, shadowfb ) {
#		HEADERS += embedded/qscreenshadow_qws.h
#		SOURCES += embedded/qscreenshadow_qws.cpp
#	}
#
#	contains( gfx-drivers, repeater ) {
#		HEADERS += embedded/qscreenrepeater_qws.h
#		SOURCES += embedded/qscreenrepeater_qws.cpp
#	}

#
# Keyboard drivers
#
	contains( kbd-drivers, sl5000 ) {
		HEADERS +=embedded/qkbdsl5000_qws.h
		SOURCES +=embedded/qkbdsl5000_qws.cpp
		!contains( kbd-drivers, tty ) {
		    kbd-drivers += tty
		}
	}

	contains( kbd-drivers, tty ) {
		HEADERS +=embedded/qkbdtty_qws.h
		SOURCES +=embedded/qkbdtty_qws.cpp
		!contains( kbd-drivers, pc101 ) {
		    kbd-drivers += pc101
		}
	}

	contains( kbd-drivers, usb ) {
		HEADERS +=embedded/qkbdusb_qws.h
		SOURCES +=embedded/qkbdusb_qws.cpp
		!contains( kbd-drivers, pc101 ) {
		    kbd-drivers += pc101
		}
	}

	contains( kbd-drivers, pc101 ) {
		HEADERS +=embedded/qkbdpc101_qws.h
		SOURCES +=embedded/qkbdpc101_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_KBD_PC101

	contains( kbd-drivers, yopy ) {
		HEADERS +=embedded/qkbdyopy_qws.h
		SOURCES +=embedded/qkbdyopy_qws.cpp
	}

	contains( kbd-drivers, vr41xx ) {
		HEADERS +=embedded/qkbdvr41xx_qws.h
		SOURCES +=embedded/qkbdvr41xx_qws.cpp
	}

#
# Mouse drivers
#
	contains( mouse-drivers, pc ) {
		HEADERS +=embedded/qmousepc_qws.h
		SOURCES +=embedded/qmousepc_qws.cpp
	}

	contains( mouse-drivers, bus ) {
		HEADERS +=embedded/qmousebus_qws.h
		SOURCES +=embedded/qmousebus_qws.cpp
	}

	contains( mouse-drivers, linuxtp ) {
		HEADERS +=embedded/qmouselinuxtp_qws.h
		SOURCES +=embedded/qmouselinuxtp_qws.cpp
	}

	contains( mouse-drivers, vr41xx ) {
		HEADERS +=embedded/qmousevr41xx_qws.h
		SOURCES +=embedded/qmousevr41xx_qws.cpp
	}

	contains( mouse-drivers, yopy ) {
		HEADERS +=embedded/qmouseyopy_qws.h
		SOURCES +=embedded/qmouseyopy_qws.cpp
	}
}
