load(qttest_p4)

SOURCES += tst_assert.cpp
QT = core

CONFIG -= debug_and_release_target
!win32:CONFIG += debug
