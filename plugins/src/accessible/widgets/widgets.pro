TEMPLATE += lib
CONFIG += qt plugin
TARGET += accwidgets
VERSION = 1.0.0
DESTDIR = ../../..
REQUIRES += accessibility

SOURCES += main.cpp \
	qaccessiblewidget.cpp \
	qaccessiblemenu.cpp

HEADERS += qaccessiblewidget.h \
	qaccessiblemenu.h
