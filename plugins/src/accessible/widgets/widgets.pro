TEMPLATE  = lib
CONFIG   += qt dll plugin
TARGET   += qtwidgets
VERSION   = 1.0.0
DESTDIR   = ../../../accessible
REQUIRES += accessibility

SOURCES  += main.cpp \
	qaccessiblewidgets.cpp \
	qaccessiblemenu.cpp

HEADERS  += qaccessiblewidgets.h \
	qaccessiblemenu.h
