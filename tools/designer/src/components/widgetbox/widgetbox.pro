TEMPLATE = lib
DESTDIR=../../../lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../uilib \
    ../../shared \
    ../formeditor

DEFINES += QT_WIDGETBOX_LIBRARY

SOURCES += widgetbox.cpp
HEADERS += widgetbox.h \
        widgetbox_global.h

RESOURCES += widgetbox.qrc

# DEFINES -= QT_COMPAT_WARNINGS
# DEFINES += QT_COMPAT

include(../../sharedcomponents.pri)
