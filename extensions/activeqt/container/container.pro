GUID 	 = {06389a79-2706-4f91-9fbd-8db657ffecd0}
TEMPLATE = lib
TARGET   = qaxcontainer

CONFIG  += qt warn_on
DESTDIR  = $$QT_BUILD_TREE/lib

INCLUDEPATH     += $$QT_SOURCE_TREE/tools/designer/interfaces

contains( QT_PRODUCT, qt-internal) {
	ACTIVEQT_H  = .
} else {
	ACTIVEQT_H  = $$QT_SOURCE_TREE/include
}

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    HEADERS         = $$ACTIVEQT_H/qaxbase.h \
		      $$ACTIVEQT_H/qaxwidget.h \
		      $$ACTIVEQT_H/qaxobject.h \
		      $$ACTIVEQT_H/qaxscript.h \
		      ../shared/types.h

    SOURCES         = qaxbase.cpp \
		      qaxwidget.cpp \
		      qaxobject.cpp \
		      qaxscript.cpp \
		      ../shared/types.cpp

    FORMS           = qactivexselect.ui
}
