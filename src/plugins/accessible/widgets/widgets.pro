TEMPLATE = lib
TARGET  += qtwidgets

CONFIG  += qt dll plugin
QT      += compat
DESTDIR  = $$QT_BUILD_TREE/plugins/accessible
VERSION  = 1.0.0

QTDIR_build:REQUIRES += "contains(QT_CONFIG, accessibility)"

SOURCES  += main.cpp \
	    simplewidgets.cpp \
#	    rangecontrols.cpp \
	    complexwidgets.cpp \
	    qaccessiblewidgets.cpp \
	    qaccessiblemenu.cpp

HEADERS  += qaccessiblewidgets.h \
	    simplewidgets.h \
#	    rangecontrols.h \
	    complexwidgets.h \
	    qaccessiblemenu.h

DEFINES += QT_COMPAT_WARNINGS
