GUID 	 = {18b63e30-29e9-497b-99ea-cb9283cac28b}
TEMPLATE = lib
TARGET  += qtwidgets

CONFIG  += qt dll plugin
DESTDIR  = ../../../accessible
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
