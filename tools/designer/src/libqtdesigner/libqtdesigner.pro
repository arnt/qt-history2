TEMPLATE=lib
TARGET=QtDesigner
CONFIG += qt dll
DESTDIR = ../../../../lib
DEFINES += QT_SDK_LIBRARY
DEFINES += QT_EXTENSION_LIBRARY

include(../extension/extension.pri)
include(../sdk/sdk.pri)
