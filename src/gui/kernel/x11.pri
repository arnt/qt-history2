x11 {
        nas {
                DEFINES +=      QT_NAS_SUPPORT
        	LIBS +=         -laudio -lXt
        }

        !xinerama:DEFINES +=    QT_NO_XINERAMA
	!xshape:DEFINES +=      QT_NO_SHAPE
	!xcursor:DEFINES +=     QT_NO_XCURSOR
	!xrandr:DEFINES +=      QT_NO_XRANDR
	!xrender:DEFINES +=     QT_NO_XRENDER
        !xft:DEFINES +=         QT_NO_XFT
	!xkb:DEFINES +=         QT_NO_XKB
        !x11sm:DEFINES +=       QT_NO_SM_SUPPORT
	xft2header:DEFINES +=   XFT2_H

	SOURCES +=      $$QT_SOURCE_TREE/src/core/codecs/qfontcncodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfonthkcodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfontjpcodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfontkrcodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfontlaocodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfonttwcodec.cpp

	HEADERS +=      $$QT_SOURCE_TREE/src/core/codecs/qfontcodecs_p.h
}
