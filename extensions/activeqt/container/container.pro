TEMPLATE = lib
TARGET   = qaxcontainer

CONFIG  += qt warn_on
DESTDIR  = $$QT_BUILD_TREE/lib

INCLUDEPATH     += $$QT_SOURCE_TREE/tools/designer/interfaces

LIBS    += -lole32 -loleaut32 -luser32 -lgdi32 -ladvapi32

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    HEADERS         = ../control/qaxaggregated.h \
                      qaxbase.h \
		      qaxwidget.h \
		      qaxobject.h \
		      qaxscript.h \
                      qaxselect.h \
		      ../shared/types.h

    SOURCES         = qaxbase.cpp \
		      qaxdump.cpp \
		      qaxwidget.cpp \
		      qaxobject.cpp \
		      qaxscript.cpp \
		      qaxscriptwrapper.cpp \
                      qaxselect.cpp \
		      ../shared/types.cpp

    FORMS           = qaxselect.ui
}
