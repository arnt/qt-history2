COMMON_FOLDER = $$PWD/../common
include(../arthurtester.pri)
CONFIG+=console moc
TEMPLATE = app
INCLUDEPATH += .

# Input
HEADERS += widgets.h interactivewidget.h
SOURCES += interactivewidget.cpp main.cpp 

contains(QT_CONFIG, opengl):QT += opengl

QT += xml svg qt3support
