unix {
	!xinerama:DEFINES += QT_NO_XINERAMA
	!xshape:DEFINES += QT_NO_SHAPE
	!xcursor:DEFINES += QT_NO_XCURSOR
	!xrandr:DEFINES += QT_NO_XRANDR
	!xrender:DEFINES += QT_NO_XRENDER
	!xftfreetype:DEFINES += QT_NO_XFTFREETYPE
	!xkb:DEFINES += QT_NO_XKB
	xft2header:DEFINES+=QT_USE_XFT2_HEADER


        xftfreetype {
	   INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/opentype
	   SOURCES += $$QT_SOURCE_TREE/src/3rdparty/opentype/ftxopentype.c
        }
}

nas {
	DEFINES     += QT_NAS_SUPPORT
	LIBS	+= -laudio -lXt
}

x11 {
	SOURCES += \
		$$QT_SOURCE_TREE/src/core/codecs/qfontcncodec.cpp \
		$$QT_SOURCE_TREE/src/core/codecs/qfonthkcodec.cpp	\
		$$QT_SOURCE_TREE/src/core/codecs/qfontjpcodec.cpp	\
		$$QT_SOURCE_TREE/src/core/codecs/qfontkrcodec.cpp	\
		$$QT_SOURCE_TREE/src/core/codecs/qfontlaocodec.cpp \
		$$QT_SOURCE_TREE/src/core/codecs/qfonttwcodec.cpp
	HEADERS += $$QT_SOURCE_TREE/src/core/codecs/qfontcodecs_p.h
}



!x11sm:DEFINES += QT_NO_SM_SUPPORT
