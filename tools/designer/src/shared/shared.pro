TEMPLATE = lib
INCLUDEPATH += .
DESTDIR = ../../lib
# DEFINES += QT_SHARED_LIBRARY
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../sdk \
    ../extension \
    ../uilib

LIBS += \
    -lQtDesigner \
    -L../../lib \
    -luilib

# Input
HEADERS += \
    shared_global.h \
    spacer.h \
    layoutinfo.h \
    connectionedit.h \
    qtundo.h \
    pluginmanager.h \
    qdesigner_formbuilder.h

SOURCES += \
    spacer.cpp \
    layoutinfo.cpp \
    connectionedit.cpp \
    qtundo.cpp \
    pluginmanager.cpp \
    qdesigner_formbuilder.cpp



