TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
QT += xml

TARGET = formeditor
DEFINES += QT_FORMEDITOR_LIBRARY

DESTDIR = ../../../lib

INCLUDEPATH += ../../lib/sdk \
    ../../shared \
    ../../uilib \
    ../../lib/extension \
    ../propertyeditor \
    ../signalsloteditor \
    ../buddyeditor

HEADERS += \
           qdesigner_resource.h \
           formwindow.h \
           formwindow_dnditem.h \
           formwindowcursor.h \
           widgetselection.h \
           formwindowmanager.h \
           orderindicator.h \
           formeditor.h \
           iconloader.h \
           formeditor_global.h \
           qlayoutwidget_propertysheet.h \
           spacer_propertysheet.h \
           default_container.h \
           default_layoutdecoration.h \
           iconcache.h \
           tool_widgeteditor.h

SOURCES += \
           qdesigner_resource.cpp \
           formwindow.cpp \
           formwindow_dnditem.cpp \
           formwindowcursor.cpp \
           widgetselection.cpp \
           formwindowmanager.cpp \
           orderindicator.cpp \
           formeditor.cpp \
           qlayoutwidget_propertysheet.cpp \
           spacer_propertysheet.cpp \
           default_container.cpp \
           default_layoutdecoration.cpp \
           iconcache.cpp \
           tool_widgeteditor.cpp

view3d {
    HEADERS += view3d.h
    SOURCES += view3d.cpp
}

MOCABLE += formwindow.cpp

QMAKE_MOD_RCC = rcc_resource
RESOURCES += formeditor.qrc

include(../../sharedcomponents.pri)
