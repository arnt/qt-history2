TEMPLATE = lib
TARGET   = QAxServer

!debug_and_release|build_pass {
   CONFIG(debug, debug|release) {
      TARGET = $$member(TARGET, 0)d
}

CONFIG  += qt warn_off staticlib
DESTDIR  = $$QT_BUILD_TREE\lib

DEFINES	+= QAX_SERVER
win32-g++:DEFINES += QT_NEEDS_QMAIN
win32-borland:DEFINES += QT_NEEDS_QMAIN

LIBS    += -luser32 -lole32 -loleaut32 -lgdi32
win32-g++:LIBS += -luuid

contains(QT_PRODUCT, .*OpenSource.*|.*Console.*) {
    message( "You are not licensed to use ActiveQt." )
} else {
    HEADERS     = qaxaggregated.h \
                  qaxbindable.h \
		  qaxfactory.h \
		  ../shared/qaxtypes.h

    SOURCES     = qaxserver.cpp \
		  qaxserverbase.cpp \
		  qaxbindable.cpp \
		  qaxfactory.cpp \
		  qaxservermain.cpp \
		  qaxserverdll.cpp \
		  qaxmain.cpp \
		  ../shared/qaxtypes.cpp
}
