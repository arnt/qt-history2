TEMPLATE = lib
TARGET   = qaxserver

CONFIG  += qt warn_off staticlib
DESTDIR  = $$QT_BUILD_TREE\lib

DEFINES	+= QAX_SERVER
LIBS    += -luser32 -lole32 -loleaut32 -lgdi32

!contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    message( "ActiveQt requires a Qt/Enterprise license." )
}
contains( QT_PRODUCT, qt-(enterprise|internal) ) {
    HEADERS     = qaxaggregated.h \
                  qaxbindable.h \
		  qaxfactory.h \
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
