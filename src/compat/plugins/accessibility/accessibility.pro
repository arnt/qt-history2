TEMPLATE = lib
TARGET  += qtcompatwidgets

CONFIG  += qt dll plugin
DESTDIR  = $$QT_BUILD_TREE/plugins/accessible
VERSION  = 1.0.0

QT += compat

SOURCES  += main.cpp \
            q3simplewidgets.cpp \
            qaccessiblecompat.cpp

HEADERS  += qaccessiblecompat.h \
            q3simplewidgets.h

