unix {
	!xinerama:DEFINES += QT_NO_XINERAMA
	!xrandr:DEFINES += QT_NO_XRANDR
	!xrender:DEFINES += QT_NO_XRENDER
	!xftfreetype:DEFINES += QT_NO_XFTFREETYPE
	!xkb:DEFINES += QT_NO_XKB
	xft2header:DEFINES+=QT_USE_XFT2_HEADER
	xftnameunparse:DEFINES += QT_NO_XFTNAMEUNPARSE
	SOURCES += $$KERNEL_CPP/qtaddons_x11.cpp \
		$$KERNEL_CPP/qscriptengine_x11.cpp \
		$$KERNEL_CPP/qtextengine_x11.cpp \
		$$KERNEL_CPP/qfontengine_x11.cpp \
		$$KERNEL_CPP/qtextlayout_x11.cpp
	HEADERS += $$KERNEL_CPP/qscriptengine_p.h \
		$$KERNEL_CPP/qtextengine_p.h \
		$$KERNEL_CPP/qfontengine_p.h \
		$$KERNEL_CPP/qtextlayout.h
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

!x11sm:DEFINES += QT_NO_SM_SUPPORT

