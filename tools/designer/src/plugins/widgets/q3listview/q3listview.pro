TEMPLATE = lib
QT += qt3support
CONFIG += plugin

include(../../plugins.pri)

HEADERS += q3listview_plugin.h \
    q3listview_extrainfo.h

SOURCES += q3listview_plugin.cpp \
    q3listview_extrainfo.cpp

CONFIG += qt warn_on qt_no_compat_warning

include(../../../sharedcomponents.pri)
