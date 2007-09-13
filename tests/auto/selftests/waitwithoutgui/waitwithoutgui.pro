load(qttest_p4)
QT -= gui
SOURCES += tst_waitwithoutgui.cpp

mac:CONFIG -= app_bundle
CONFIG -= debug_and_release_target

DEFINES += QT_USE_USING_NAMESPACE

