TEMPLATE=lib
TARGET=QtDesigner
QT += xml
CONFIG += qt dll
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin

DEFINES += \
    QT_SDK_LIBRARY \
    QT_EXTENSION_LIBRARY \
    QT_UILIB_LIBRARY \
    QT_SHARED_LIBRARY

include(extension/extension.pri)
include(sdk/sdk.pri)
include(uilib/uilib.pri)
include(shared/shared.pri)

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

include(../sharedcomponents.pri)
