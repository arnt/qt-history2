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
    spacer_widget.h \
    layoutinfo.h \
    layout.h \
    connectionedit.h \
    qtundo.h \
    pluginmanager.h \
    metadatabase.h \
    qdesigner_command.h \
    qdesigner_formbuilder.h \
    qdesigner_taskmenu.h \
    qdesigner_widget.h \
    qdesigner_propertysheet.h \
    qdesigner_integration.h \
    invisible_widget.h \
    qlayout_widget.h \
    tree_widget.h \
    sheet_delegate.h \
    qdesigner_promotedwidget.h \
    qdesigner_customwidget.h \
    qdesigner_stackedbox.h \
    qdesigner_tabwidget.h \
    qdesigner_toolbox.h \
    qdesigner_dnditem.h \
    widgetfactory.h \
    widgetdatabase.h \
    promotetocustomwidgetdialog.h \
    resourcefile.h

SOURCES += \
    spacer_widget.cpp \
    layoutinfo.cpp \
    layout.cpp \
    connectionedit.cpp \
    qtundo.cpp \
    pluginmanager.cpp \
    qdesigner_command.cpp \
    qdesigner_formbuilder.cpp \
    qdesigner_taskmenu.cpp \
    qdesigner_widget.cpp \
    qdesigner_propertysheet.cpp \
    qdesigner_integration.cpp \
    qdesigner_dnditem.cpp \
    invisible_widget.cpp \
    qlayout_widget.cpp \
    tree_widget.cpp \
    sheet_delegate.cpp \
    metadatabase.cpp \
    qdesigner_promotedwidget.cpp \
    qdesigner_customwidget.cpp \
    qdesigner_stackedbox.cpp \
    qdesigner_tabwidget.cpp \
    qdesigner_toolbox.cpp \
    widgetfactory.cpp \
    widgetdatabase.cpp \
    promotetocustomwidgetdialog.cpp \
    resourcefile.cpp

FORMS += promotetocustomwidgetdialog.ui

include(../sharedcomponents.pri)
