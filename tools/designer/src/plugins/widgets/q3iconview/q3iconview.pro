TEMPLATE = lib
QT += qt3support

HEADERS += q3iconview_plugin.h \
    q3iconview_extrainfo.h

SOURCES += q3iconview_plugin.cpp \
    q3iconview_extrainfo.cpp

CONFIG += qt warn_on qt_no_compat_warning

include(../../plugins.pri)
