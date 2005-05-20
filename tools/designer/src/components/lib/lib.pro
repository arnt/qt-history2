TEMPLATE = lib
TARGET = QtDesignerComponents
CONFIG += qt dll debug_and_release
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

INCLUDEPATH += . ..

LIBS += -L$$QT_BUILD_TREE/tools/designer/lib \
    -lQtDesigner \
    -lpropertyeditor \
    -lobjectinspector \
    -lsignalsloteditor \
    -lformeditor \
    -lwidgetbox \
    -lbuddyeditor \
    -ltaskmenu \
    -ltabordereditor \
    -lresourceeditor \


include(../../sharedcomponents.pri)
include(../component.pri)
