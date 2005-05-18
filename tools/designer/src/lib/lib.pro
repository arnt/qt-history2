TEMPLATE=lib
TARGET=QtDesigner
QT += xml
CONFIG += qt dll debug_and_release
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

VERSION = 4.0.0
QMAKE_TARGET_COMPANY = Trolltech AS
QMAKE_TARGET_PRODUCT = Designer
QMAKE_TARGET_DESCRIPTION = Graphical user interface designer.
QMAKE_TARGET_COPYRIGHT = Copyright (c) 2003-2005 Trolltech

DEFINES += \
    QT_SDK_LIBRARY \
    QT_EXTENSION_LIBRARY \
    QT_UILIB_LIBRARY \
    QT_SHARED_LIBRARY \
    QT_PROPERTYEDITOR_LIBRARY

include(extension/extension.pri)
include(sdk/sdk.pri)
include(uilib/uilib.pri)
include(shared/shared.pri)
PRECOMPILED_HEADER=lib_pch.h

### include(propertyeditor/propertyeditor.pri)

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target
include(../components/component.pri)
include(../sharedcomponents.pri)
