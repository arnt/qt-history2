TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT -= gui
CONFIG += qdbus

# Input
HEADERS += complexpong.h
SOURCES += complexpong.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qdbus/complexpingpong
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qdbus/complexpingpong
INSTALLS += target sources
DEFINES += QT_USE_USING_NAMESPACE
