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
    ../signalsloteditor

HEADERS += qdesigner_widget.h \
           qdesigner_tabwidget.h \
           qdesigner_stackedbox.h \
           qdesigner_toolbox.h \
           qdesigner_resource.h \
           qdesigner_customwidget.h \
           formwindow.h \
           formwindowcursor.h \
           sizehandle.h \
           widgetfactory.h \
           layout.h \
           metadatabase.h \
           widgetdatabase.h \
           command.h \
           formwindowmanager.h \
           orderindicator.h \
           formeditor.h \
           iconloader.h \
           formeditor_global.h \
           default_propertysheet.h \
           qlayoutwidget_propertysheet.h \
           spacer_propertysheet.h \
           default_container.h \
           layoutdecoration.h \
           default_layoutdecoration.h

SOURCES += qdesigner_widget.cpp \
           qdesigner_tabwidget.cpp \
           qdesigner_stackedbox.cpp \
           qdesigner_toolbox.cpp \
           qdesigner_resource.cpp \
           qdesigner_customwidget.cpp \
           formwindow.cpp \
           formwindowcursor.cpp \
           sizehandle.cpp \
           widgetfactory.cpp \
           layout.cpp \
           metadatabase.cpp \
           widgetdatabase.cpp \
           command.cpp \
           formwindowmanager.cpp \
           orderindicator.cpp \
           formeditor.cpp \
           default_propertysheet.cpp \
           qlayoutwidget_propertysheet.cpp \
           spacer_propertysheet.cpp \
           default_container.cpp \
           default_layoutdecoration.cpp


MOCABLE += formwindow.cpp

QMAKE_MOD_RCC = rcc_resource
RESOURCES += formeditor.qrc

include(../../sharedcomponents.pri)
