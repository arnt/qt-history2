TARGET  = qtcompatwidgets
CONFIG += qt_no_compat_warning
include(../../qpluginbase.pri)

DESTDIR  = $$QT_BUILD_TREE/plugins/accessible
VERSION  = 1.0.0

QTDIR_build:REQUIRES += "contains(QT_CONFIG, accessibility)"

QT += compat

SOURCES  += main.cpp \
            q3simplewidgets.cpp \
            q3complexwidgets.cpp \
            qaccessiblecompat.cpp

HEADERS  += qaccessiblecompat.h \
            q3complexwidgets.h \
            q3simplewidgets.h

