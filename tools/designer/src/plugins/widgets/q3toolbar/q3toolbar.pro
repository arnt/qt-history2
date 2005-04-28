TEMPLATE = lib
QT += qt3support

HEADERS += q3toolbar_plugin.h \
    q3toolbar_extrainfo.h

SOURCES += q3toolbar_plugin.cpp \
    q3toolbar_extrainfo.cpp

CONFIG += qt warn_on qt_no_compat_warning

include(../../plugins.pri)
