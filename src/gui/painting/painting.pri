# Qt gui library, paint module

HEADERS += \
	painting/qbrush.h \
	painting/qcolor.h \
	painting/qcolor_p.h \
	painting/qcolormap.h \
	painting/qdrawutil.h \
	painting/qline.h \
	painting/qpaintdevice.h \
	painting/qpaintdevicedefs.h \
	painting/qpaintdevicemetrics.h \
	painting/qpaintengine.h \
	painting/qpainter.h \
	painting/qpainter_p.h \
	painting/qpainterpath.h \
	painting/qpainterpath_p.h \
	painting/qpen.h \
	painting/qpoint.h \
	painting/qpointarray.h \
	painting/qpolygon.h \
	painting/qpolygonscanner.h \
	painting/qprinter.h \
	painting/qprinter_p.h \
	painting/qrect.h \
	painting/qregion.h \
	painting/qsize.h \
	painting/qmatrix.h \
	painting/qwmatrix.h


SOURCES += \
	painting/qbrush.cpp \
	painting/qcolor.cpp \
	painting/qcolor_p.cpp \
	painting/qdrawutil.cpp \
	painting/qline.cpp \
	painting/qpaintdevicemetrics.cpp \
	painting/qpaintengine.cpp \
	painting/qpainter.cpp \
	painting/qpainterpath.cpp \
	painting/qpen.cpp \
	painting/qpoint.cpp \
	painting/qpointarray.cpp \
	painting/qpolygon.cpp \
	painting/qpolygonscanner.cpp \
	painting/qprinter.cpp \
	painting/qrect.cpp \
	painting/qregion.cpp \
	painting/qsize.cpp \
	painting/qmatrix.cpp

win32 {
	HEADERS += \
		painting/qpaintengine_win.h \
		painting/qpaintengine_win_p.h \
		painting/qprintengine_win.h \
		painting/qprintengine_win_p.h
		

 	SOURCES += \
		painting/qcolor_win.cpp \
		painting/qcolormap_win.cpp \
		painting/qpaintdevice_win.cpp \
		painting/qpaintengine_win.cpp \
		painting/qprintengine_win.cpp \
		painting/qregion_win.cpp
	LIBS += -lmsimg32
}

wince-* {
	SOURCES -= painting/qregion_win.cpp
	SOURCES += painting/qregion_wce.cpp
}


unix:x11 {
	HEADERS += \
		painting/qpaintengine_x11.h \
		painting/qpaintengine_x11_p.h

	SOURCES += \
		painting/qcolormap_x11.cpp \
		painting/qpaintdevice_x11.cpp \
		painting/qpaintengine_x11.cpp
}

!embedded:!x11:mac {
	HEADERS += \
		painting/qpaintengine_mac.h \
		painting/qpaintengine_mac_p.h \
		painting/qprintengine_mac.h \
		painting/qprintengine_mac_p.h

	SOURCES += \
		painting/qcolormap_mac.cpp \
		painting/qpaintdevice_mac.cpp \
		painting/qpaintengine_mac.cpp \
		painting/qprintengine_mac.cpp
} else:unix {
	HEADERS	+= \
		painting/qprintengine_ps.h

	SOURCES += \
		painting/qprintengine_ps.cpp
}

unix:SOURCES += painting/qregion_unix.cpp


embedded {
	HEADERS += \
		painting/qpaintengine_qws.h \
		painting/qpaintengine_qws_p.h

	SOURCES += \
		painting/qcolormap_qws.cpp \
		painting/qpaintdevice_qws.cpp \
		painting/qpaintengine_qws.cpp
}
