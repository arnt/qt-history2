TEMPLATE = app
CONFIG -= moc
mac:CONFIG -= resource_fork
win32:CONFIG += console
QT = core
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../../bin

# Input
SOURCES += main.cpp
