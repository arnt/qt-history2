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
    layout.h \
    connectionedit.h \
    qtundo.h \
    pluginmanager.h \
    qdesigner_command.h \
    qdesigner_formbuilder.h \
    qdesigner_taskmenu.h \
    qdesigner_widget.h \
    default_propertysheet.h \
    invisible_widget.h \
    treewidget.h \
    sheet_delegate.h

SOURCES += \
    spacer.cpp \
    layoutinfo.cpp \
    layout.cpp \
    connectionedit.cpp \
    qtundo.cpp \
    pluginmanager.cpp \
    qdesigner_command.cpp \
    qdesigner_formbuilder.cpp \
    qdesigner_taskmenu.cpp \
    qdesigner_widget.cpp \
    default_propertysheet.cpp \
    invisible_widget.cpp \
    treewidget.cpp \
    sheet_delegate.cpp

include(../sharedcomponents.pri)
