TEMPLATE  = lib
CONFIG   += qt dll plugin
TARGET   += qtwidgets
VERSION   = 1.0.0
DESTDIR   = ../../../accessible
REQUIRES += accessibility

SOURCES  += main.cpp \
	    simplewidgets.cpp \
	    rangecontrols.cpp \
	    qaccessiblewidgets.cpp \
	    qaccessiblemenu.cpp

HEADERS  += qaccessiblewidgets.h \
	    simplewidgets.h \
	    rangecontrols.h \
	    qaccessiblemenu.h
