TEMPLATE = lib
QT += qt3support

HEADERS += q3widgetstack_container.h \
    q3widgetstack_plugin.h

SOURCES += q3widgetstack_container.cpp \
    q3widgetstack_plugin.cpp

CONFIG += qt warn_on qt_no_compat_warning

include(../../plugins.pri)
