TEMPLATE = lib
CONFIG += qt plugin

win32|mac:!win32-msvc:!macx-xcode:CONFIG += debug_and_release
TARGET = $$qtLibraryTarget($$TARGET)
contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols

include(../qt_targets.pri)
