# Qt gui library, paint module

HEADERS += \
	painting/q3painter.h \
	painting/qbezier_p.h \
	painting/qbrush.h \
	painting/qcolor.h \
	painting/qcolor_p.h \
	painting/qcolormap.h \
	painting/qdrawutil.h \
	painting/qline.h \
	painting/qpaintdevice.h \
	painting/qpaintengine.h \
	painting/qpainter.h \
	painting/qpainter_p.h \
	painting/qpainterpath.h \
	painting/qpainterpath_p.h \
	painting/qpen.h \
	painting/qpolygon.h \
	painting/qprinter.h \
	painting/qprinter_p.h \
	painting/qregion.h \
        painting/qstylepainter.h \
	painting/qmatrix.h \
	painting/qwmatrix.h


SOURCES += \
	painting/q3painter.cpp \
	painting/qbezier.cpp \
	painting/qbrush.cpp \
	painting/qcolor.cpp \
	painting/qcolor_p.cpp \
	painting/qdrawutil.cpp \
	painting/qline.cpp \
	painting/qpaintengine.cpp \
	painting/qpainter.cpp \
	painting/qpainterpath.cpp \
	painting/qpen.cpp \
	painting/qpolygon.cpp \
	painting/qprinter.cpp \
        painting/qstylepainter.cpp \
	painting/qregion.cpp \
	painting/qmatrix.cpp

win32 {
	HEADERS += \
		painting/qpaintengine_win_p.h \
		painting/qprintengine_win_p.h


 	SOURCES += \
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
		painting/qpaintengine_x11_p.h

	SOURCES += \
		painting/qcolormap_x11.cpp \
		painting/qpaintdevice_x11.cpp \
		painting/qpaintengine_x11.cpp
}

!embedded:!x11:mac {
	HEADERS += \
		painting/qpaintengine_mac_p.h \
		painting/qprintengine_mac_p.h

	SOURCES += \
		painting/qcolormap_mac.cpp \
		painting/qpaintdevice_mac.cpp \
		painting/qpaintengine_mac.cpp \
		painting/qprintengine_mac.cpp
} else:unix {
	HEADERS	+= \
		painting/qprintengine_ps_p.h

	SOURCES += \
		painting/qprintengine_ps.cpp
}

unix:SOURCES += painting/qregion_unix.cpp


embedded {
	HEADERS += \
		painting/qpaintengine_qws_p.h

	SOURCES += \
		painting/qcolormap_qws.cpp \
		painting/qpaintdevice_qws.cpp \
		painting/qpaintengine_qws.cpp
}
