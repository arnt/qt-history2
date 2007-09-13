TEMPLATE = app
TARGET = pong
DEPENDPATH += .
INCLUDEPATH += .
QT -= gui
CONFIG += qdbus

# Input
HEADERS += ping-common.h pong.h
SOURCES += pong.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qdbus/pingpong
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qdbus/pingpong
INSTALLS += target sources
DEFINES += QT_USE_USING_NAMESPACE
