load(qttest_p4)
SOURCES += tst_datetime.cpp
QT = core

mac:CONFIG -= app_bundle
CONFIG -= debug_and_release_target

DEFINES += QT_USE_USING_NAMESPACE

