TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
QT -= gui
CONFIG += qdbus
win32:CONFIG += console

# Input
SOURCES += listnames.cpp

# install
target.path = $$[QT_INSTALL_EXAMPLES]/qdbus/listnames
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.pro
sources.path = $$[QT_INSTALL_EXAMPLES]/qdbus/listnames
INSTALLS += target sources

DEFINES += QT_USE_USING_NAMESPACE
