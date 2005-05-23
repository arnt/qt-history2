TEMPLATE = lib
TARGET = QtDesignerComponents
CONFIG += qt dll debug_and_release depend_prl
DESTDIR = $$QT_BUILD_TREE/lib
DLLDESTDIR = $$QT_BUILD_TREE/bin

VERSION = 4.0.0
QMAKE_TARGET_COMPANY = Trolltech AS
QMAKE_TARGET_PRODUCT = Designer
QMAKE_TARGET_DESCRIPTION = Graphical user interface designer.
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2003-2005 Trolltech

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

HEADERS += qdesigner_components.h \
	   qdesigner_components_global.h

SOURCES += qdesigner_components.cpp \
    qdesigner_plugins.cpp

DEFINES += QDESIGNER_COMPONENTS_LIBRARY

INCLUDEPATH += . .. \
    $$QT_SOURCE_TREE/tools/designer/src/lib/sdk \
    $$QT_SOURCE_TREE/tools/designer/src/lib/extension \
    $$QT_SOURCE_TREE/tools/designer/src/lib/uilib \
    $$QT_SOURCE_TREE/tools/designer/src/lib/shared

include(../propertyeditor/propertyeditor.pri)
include(../objectinspector/objectinspector.pri)
include(../signalsloteditor/signalsloteditor.pri)
include(../formeditor/formeditor.pri)
include(../widgetbox/widgetbox.pri)
include(../buddyeditor/buddyeditor.pri)
include(../taskmenu/taskmenu.pri)
include(../tabordereditor/tabordereditor.pri)
include(../resourceeditor/resourceeditor.pri)

PRECOMPILED_HEADER= lib_pch.h

LIBS += -lQtDesigner

include(../../sharedcomponents.pri)
include(../component.pri)
