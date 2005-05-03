TEMPLATE=lib
TARGET=QtDesigner
QT += xml
CONFIG += qt dll debug_and_release
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

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
