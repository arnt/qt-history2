TEMPLATE = lib
CONFIG += qt
CONFIG += staticlib
QT += xml

TARGET = formeditor
DEFINES += QT_FORMEDITOR_LIBRARY

DESTDIR = ../../../lib

INCLUDEPATH += ../../lib/sdk \
    ../../lib/shared \
    ../../lib/uilib \
    ../../lib/extension \
    ../propertyeditor \
    ../signalsloteditor \
    ../buddyeditor

PRECOMPILED_HEADER=formeditor_pch.h
HEADERS += \
           qdesigner_resource.h \
           formwindow.h \
           formwindow_widgetstack.h \
           formwindow_dnditem.h \
           formwindowcursor.h \
           widgetselection.h \
           formwindowmanager.h \
           formeditor.h \
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
           formwindow_widgetstack.cpp \
           formwindow_dnditem.cpp \
           formwindowcursor.cpp \
           widgetselection.cpp \
           formwindowmanager.cpp \
           formeditor.cpp \
           qlayoutwidget_propertysheet.cpp \
           spacer_propertysheet.cpp \
           default_container.cpp \
           default_layoutdecoration.cpp \
           tool_widgeteditor.cpp

MOCABLE += formwindow.cpp

RESOURCES += formeditor.qrc

include(../component.pri)
