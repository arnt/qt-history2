TEMPLATE = app
CONFIG -= moc
mac:CONFIG -= app_bundle
win32:CONFIG += console
QT = core
DEPENDPATH += .
INCLUDEPATH += .
DESTDIR = ../../bin

# Input
SOURCES += main.cpp
