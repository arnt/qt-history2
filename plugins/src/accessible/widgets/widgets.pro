TEMPLATE  = lib
CONFIG   += qt dll plugin
TARGET   += qtwidgets
VERSION   = 1.0.0
DESTDIR   = ../../../accessible
REQUIRES += accessibility

SOURCES  += main.cpp \
	qaccessiblewidget.cpp \
	qaccessiblemenu.cpp

HEADERS  += qaccessiblewidget.h \
	qaccessiblemenu.h
