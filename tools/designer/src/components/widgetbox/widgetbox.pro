TEMPLATE = lib
DESTDIR=../../../lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../lib/uilib \
    ../../lib/shared \
    ../formeditor

DEFINES += QT_WIDGETBOX_LIBRARY

SOURCES += widgetbox.cpp \
            widgetbox_dnditem.cpp
HEADERS += widgetbox.h \
            widgetbox_global.h \
	    widgetbox_dnditem.h

RESOURCES += widgetbox.qrc

# DEFINES -= QT_COMPAT_WARNINGS
# DEFINES += QT_COMPAT

include(../component.pri)
