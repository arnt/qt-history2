unix {
	xinerama:LIBS += -lXinerama
	!xinerama:DEFINES += QT_NO_XINERAMA
	xrender:LIBS += -lXrender
	!xrender:DEFINES += QT_NO_XRENDER
	xftfreetype:LIBS += -lXft
	!xftfreetype:DEFINES += QT_NO_XFTFREETYPE
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

sm:CONFIG += x11sm
!x11sm:DEFINES += QT_NO_SM_SUPPORT

