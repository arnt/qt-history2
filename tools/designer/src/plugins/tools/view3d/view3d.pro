
TEMPLATE = lib 
QT	+= opengl
CONFIG  += qt warn_on plugin
DESTDIR =
TARGET = view3d

include(../../plugins.pri)
include(../../../sharedcomponents.pri)
include(../../../lib/sdk/sdk.pri)

# Input
SOURCES += view3d.cpp
HEADERS += view3d.h

