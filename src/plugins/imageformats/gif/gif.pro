TARGET  = qgif
include(../../qpluginbase.pri)

DESTDIR  = $$QT_BUILD_TREE/plugins/imageformats

VERSION = 1.0.0

HEADERS += qgifhandler.h

SOURCES += main.cpp \
           qgifhandler.cpp

target.path += $$[QT_INSTALL_PLUGINS]/imageformats
INSTALLS += target

QT += qt3support