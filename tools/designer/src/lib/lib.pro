TEMPLATE=lib
TARGET=QtDesigner
CONFIG += qt dll
DESTDIR = ../../../../lib
DLLDESTDIR = ../../../../bin
DEFINES += QT_SDK_LIBRARY
DEFINES += QT_EXTENSION_LIBRARY

include(extension/extension.pri)
include(sdk/sdk.pri)

target.path=$$libs.path
INSTALLS        += target

include(../sharedcomponents.pri)
