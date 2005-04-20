TEMPLATE = lib
QT += qt3support
CONFIG += plugin designer

HEADERS += q3listview_plugin.h \
    q3listview_extrainfo.h

SOURCES += q3listview_plugin.cpp \
    q3listview_extrainfo.cpp

CONFIG += qt warn_on qt_no_compat_warning
