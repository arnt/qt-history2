TEMPLATE = lib
DESTDIR=../../../lib
QT += xml
CONFIG += qt
CONFIG += staticlib

INCLUDEPATH += \
    ../../lib/sdk \
    ../../lib/extension \
    ../../lib/uilib \
    ../../lib/shared

DEFINES += QT_RESOURCEEDITOR_LIBRARY

SOURCES += resourceeditor.cpp \
            resourceeditor_instance.cpp \
            resourceeditor_plugin.cpp \
            resourceeditor_tool.cpp
HEADERS += resourceeditor.h \
            resourceeditor_global.h \
            resourceeditor_plugin.h \
            resourceeditor_tool.h

# DEFINES -= QT_COMPAT_WARNINGS
# DEFINES += QT_COMPAT

include(../component.pri)
