unix {
	!xinerama:DEFINES += QT_NO_XINERAMA
	!xrender:DEFINES += QT_NO_XRENDER
	!xftfreetype:DEFINES += QT_NO_XFTFREETYPE
	!xkb:DEFINES += QT_NO_XKB
	xftnameunparse:DEFINES += QT_NO_XFTNAMEUNPARSE
	SOURCES += $$KERNEL_CPP/qtaddons_x11.cpp
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

!x11sm:DEFINES += QT_NO_SM_SUPPORT

