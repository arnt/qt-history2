TEMPLATE = lib
INCLUDEPATH += .
DESTDIR = ../../lib
# DEFINES += QT_SHARED_LIBRARY
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../lib/sdk \
    ../lib/extension \
    ../uilib

LIBS += \
    -lQtDesigner \
    -L$(QTDIR)/tools/designer/lib \
    -luilib

# Input
HEADERS += \
    shared_global.h \
    spacer.h \
    layoutinfo.h \
    connectionedit.h \
    qtundo.h \
    pluginmanager.h \
    qdesigner_formbuilder.h \
    default_propertysheet.h \
    treewidget.h

SOURCES += \
    spacer.cpp \
    layoutinfo.cpp \
    connectionedit.cpp \
    qtundo.cpp \
    pluginmanager.cpp \
    qdesigner_formbuilder.cpp \
    default_propertysheet.cpp \
    treewidget.cpp

include(../sharedcomponents.pri)
