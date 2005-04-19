TEMPLATE = lib
QT += qt3support
CONFIG += plugin

include(../../plugins.pri)
LIBS += -lQtDesigner

SOURCES += plugin.cpp
CONFIG += qt warn_on qt_no_compat_warning

include(../../../sharedcomponents.pri)
