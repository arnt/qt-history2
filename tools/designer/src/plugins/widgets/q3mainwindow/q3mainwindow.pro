TEMPLATE = lib
QT += qt3support

HEADERS += q3mainwindow_container.h \
    q3mainwindow_plugin.h

SOURCES += q3mainwindow_container.cpp \
    q3mainwindow_plugin.cpp

CONFIG += qt warn_on qt_no_compat_warning

include(../../plugins.pri)
