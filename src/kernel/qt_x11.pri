unix {
	!xinerama:DEFINES += QT_NO_XINERAMA
	!xrender:DEFINES += QT_NO_XRENDER
	!xftfreetype:DEFINES += QT_NO_XFTFREETYPE
	xftnameunparse:DEFINES += QT_NO_XFTNAMEUNPARSE
	xftnameunparse:SOURCES += $$KERNEL_CPP/xftnameunparse.c
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

!x11sm:DEFINES += QT_NO_SM_SUPPORT

