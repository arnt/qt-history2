TEMPLATE = lib
TARGET  += qtcompatwidgets

CONFIG  += qt dll plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/accessible
VERSION  = 1.0.0

QTDIR_build:REQUIRES += accessibility

QT += compat

SOURCES  += main.cpp \
            qaccessiblecompat.cpp

HEADERS  += qaccessiblecompat.h
