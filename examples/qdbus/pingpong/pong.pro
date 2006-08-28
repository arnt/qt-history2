TEMPLATE = app
TARGET = pong
DEPENDPATH += .
INCLUDEPATH += .
QT -= gui
CONFIG += qdbus

# Input
HEADERS += ping-common.h pong.h
SOURCES += pong.cpp
