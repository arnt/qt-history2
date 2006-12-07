TEMPLATE = app
TARGET = undo
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += debug

HEADERS += document.h mainwindow.h commands.h
SOURCES += document.cpp main.cpp mainwindow.cpp commands.cpp
FORMS += mainwindow.ui
RESOURCES += undo.qrc
