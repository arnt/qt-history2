TEMPLATE = lib
TARGET   = qaxcontainer

CONFIG  += qt warn_on
DESTDIR  = $$QT_BUILD_TREE/lib

INCLUDEPATH     += $$QT_SOURCE_TREE/tools/designer/interfaces

LIBS    += -lole32 -loleaut32 -luser32 -lgdi32

contains( QT_PRODUCT, qt-internal) {
	ACTIVEQT_H  = .
        ACTIVEQT_AGGH  = ../control
} else {
	ACTIVEQT_H  = $$QT_SOURCE_TREE/include
        ACTIVEQT_AGGH  = $$QT_SOURCE_TREE/include
}

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    HEADERS         = $$ACTIVEQT_AGGH/qaxaggregated.h \
                      $$ACTIVEQT_H/qaxbase.h \
		      $$ACTIVEQT_H/qaxwidget.h \
		      $$ACTIVEQT_H/qaxobject.h \
		      $$ACTIVEQT_H/qaxscript.h \
		      ../shared/types.h

    SOURCES         = qaxbase.cpp \
		      qaxdump.cpp \
		      qaxwidget.cpp \
		      qaxobject.cpp \
		      qaxscript.cpp \
		      qaxscriptwrapper.cpp \
		      ../shared/types.cpp

    FORMS           = qactivexselect.ui
}
