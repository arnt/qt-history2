TEMPLATE = lib
DESTDIR=../../../lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../lib/uilib \
    ../../lib/shared

DEFINES += QT_RESOURCEEDITOR_LIBRARY

SOURCES += resourceeditor.cpp
HEADERS += resourceeditor.h \
            resourceeditor_global.h
FORMS += resourceeditor.ui

# DEFINES -= QT_COMPAT_WARNINGS
# DEFINES += QT_COMPAT

include(../component.pri)
