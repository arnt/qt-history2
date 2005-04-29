TEMPLATE = lib
QT += qt3support

HEADERS += q3wizard_container.h \
    q3wizard_plugin.h

SOURCES += q3wizard_container.cpp \
    q3wizard_plugin.cpp

CONFIG += qt warn_on qt_no_compat_warning

include(../../plugins.pri)
