# Qt/Embedded Drivers

embedded {
	EMBEDDED_P = embedded

	HEADERS += $$EMBEDDED_P/qgfxdriverinterface_p.h \
		    $$EMBEDDED_H/qgfxdriverplugin_qws.h \
		    $$EMBEDDED_H/qgfxdriverfactory_qws.h \
		    $$EMBEDDED_H/qgfxlinuxfb_qws.h

	SOURCES += $$EMBEDDED_CPP/qgfxdriverplugin_qws.cpp \
		    $$EMBEDDED_CPP/qgfxdriverfactory_qws.cpp \
		    $$EMBEDDED_CPP/qgfxlinuxfb_qws.cpp

	contains( gfx-drivers, qvfb ) {
		HEADERS += $$EMBEDDED_H/qgfxvfb_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvfb_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VFB

	contains( gfx-drivers, vnc ) {
		HEADERS += $$EMBEDDED_H/qgfxvnc_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvnc_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VNC

	contains( gfx-drivers, vga16 ) {
		HEADERS += $$EMBEDDED_H/qgfxvga16_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvga16_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VGA16

	contains( gfx-drivers, transformed ) {
		HEADERS += $$EMBEDDED_H/qgfxtransformed_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxtransformed_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_TRANSFORMED

	contains( gfx-drivers, mach64 ) {
		HEADERS += $$EMBEDDED_H/qgfxmach64_qws.h \
			   $$EMBEDDED_H/qgfxmach64defs_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxmach64_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MACH64

	contains( gfx-drivers, voodoo ) {
		HEADERS += $$EMBEDDED_H/qgfxvoodoo_qws.h \
			   $$EMBEDDED_H/qgfxvoodoodefs_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxvoodoo_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_VOODOO3

	contains( gfx-drivers, matrox ) {
		HEADERS += $$EMBEDDED_H/qgfxmatrox_qws.h \
			   $$EMBEDDED_H/qgfxmatroxdefs_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxmatrox_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_MATROX

	contains( gfx-drivers, shadowfb ) {
		HEADERS += $$EMBEDDED_H/qgfxshadow_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxshadow_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_SHADOWFB

	contains( gfx-drivers, repeater ) {
		HEADERS += $$EMBEDDED_H/qgfxrepeater_qws.h
		SOURCES += $$EMBEDDED_CPP/qgfxrepeater_qws.cpp
	}
	else:DEFINES += QT_NO_QWS_REPEATER
}

