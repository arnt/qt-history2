TEMPLATE = lib
TARGET  += qtwidgets

QCONFIG += compat
CONFIG  += qt dll plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/accessible
VERSION  = 1.0.0

DEFINES += QT_COMPAT_WARNINGS

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
