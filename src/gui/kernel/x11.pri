x11 {
        contains(QT_CONFIG, nas) {
                DEFINES +=      QT_NAS_SUPPORT
        	LIBS +=         -laudio -lXt
        }

        !contains(QT_CONFIG, xinerama):DEFINES +=    QT_NO_XINERAMA
	!contains(QT_CONFIG, xshape):DEFINES +=      QT_NO_SHAPE
	!contains(QT_CONFIG, xcursor):DEFINES +=     QT_NO_XCURSOR
	!contains(QT_CONFIG, xrandr):DEFINES +=      QT_NO_XRANDR
	!contains(QT_CONFIG, xrender):DEFINES +=     QT_NO_XRENDER
        !contains(QT_CONFIG, xft):DEFINES +=         QT_NO_XFT
        else:QMAKE_INCDIR_X11 *= $$FREETYPE2_INCDIR
	!contains(QT_CONFIG, xkb):DEFINES +=         QT_NO_XKB
        !contains(QT_CONFIG, x11sm):DEFINES +=       QT_NO_SM_SUPPORT
	contains(QT_CONFIG, xft2header):DEFINES +=   XFT2_H

	SOURCES +=      $$QT_SOURCE_TREE/src/core/codecs/qfontcncodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfonthkcodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfontjpcodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfontkrcodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfontlaocodec.cpp \
                        $$QT_SOURCE_TREE/src/core/codecs/qfonttwcodec.cpp

	HEADERS +=      $$QT_SOURCE_TREE/src/core/codecs/qfontcodecs_p.h
}
