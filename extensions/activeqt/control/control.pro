TEMPLATE = lib
TARGET   = qaxserver

CONFIG  += qt warn_off staticlib
DESTDIR  = $$QT_BUILD_TREE\lib

DEFINES	+= QAX_SERVER

contains( QT_PRODUCT, qt-internal) {
	ACTIVEQT_H  = .
} else {
	ACTIVEQT_H  = $$QT_SOURCE_TREE/include
}

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    HEADERS     = $$ACTIVEQT_H/qaxaggregated.h \
                  $$ACTIVEQT_H/qaxbindable.h \
		  $$ACTIVEQT_H/qaxfactory.h \
		  ../shared/types.h

    SOURCES     = qaxserver.cpp \
		  qaxserverbase.cpp \
		  qaxbindable.cpp \
		  qaxfactory.cpp \
		  qaxservermain.cpp \
		  qaxserverdll.cpp \
		  qaxmain.cpp \
		  ../shared/types.cpp
}
