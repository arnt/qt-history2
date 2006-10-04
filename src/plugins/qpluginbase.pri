TEMPLATE = lib
CONFIG += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

include(../qt_targets.pri)
