unix {
	xinerama:LIBS += -lXinerama
	!xinerama:DEFINES += QT_NO_XINERAMA
	xrender:LIBS += -lXrender
	!xrender:DEFINES += QT_NO_XRENDER
	xftfreetype:LIBS += -lXft
	!xftfreetype:DEFINES += QT_NO_XFTFREETYPE
	xftnameunparse:DEFINES += QT_NO_XFTNAMEUNPARSE
	xftnameunparse:SOURCES += $$KERNEL_CPP/xftnameunparse.c
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

!x11sm:DEFINES += QT_NO_SM_SUPPORT

