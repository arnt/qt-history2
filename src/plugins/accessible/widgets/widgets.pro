TEMPLATE = lib
TARGET  += qtwidgets

CONFIG  += qt dll plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/accessible
VERSION  = 1.0.0

QTDIR_build:REQUIRES += accessibility

SOURCES  += main.cpp \
	    simplewidgets.cpp \
	    rangecontrols.cpp \
	    containers.cpp \
	    complexwidgets.cpp \
	    qaccessiblewidgets.cpp \
	    qaccessiblemenu.cpp

HEADERS  += qaccessiblewidgets.h \
	    simplewidgets.h \
	    rangecontrols.h \
	    containers.h \
	    complexwidgets.h \
	    qaccessiblemenu.h

DEFINES += QT_COMPAT_WARNINGS
