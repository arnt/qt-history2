TARGET = qaxwidget
TEMPLATE = lib

include($$QT_SOURCE_TREE/tools/designer/src/plugins/plugins.pri)

# Input
SOURCES += plugin.cpp
CONFIG += qt warn_on qaxcontainer

INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/src/lib/shared