TEMPLATE=lib
TARGET=QtDesigner
CONFIG += qt dll
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin
DEFINES += QT_SDK_LIBRARY
DEFINES += QT_EXTENSION_LIBRARY

include(extension/extension.pri)
include(sdk/sdk.pri)

target.path=$$[QT_INSTALL_LIBS]
INSTALLS        += target

include(../sharedcomponents.pri)
