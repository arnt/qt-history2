load(qttest_p4)
TEMPLATE = app
win32:TARGET = ../clickLock
!win32:TARGET = clickLock

QT += qt3support
DEPENDPATH += .
INCLUDEPATH += .

# Input
SOURCES += main.cpp

DEFINES += QT_USE_USING_NAMESPACE

