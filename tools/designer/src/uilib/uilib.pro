TEMPLATE = lib
DEPENDPATH += .
INCLUDEPATH += . 
DESTDIR= ../../lib
CONFIG += qt
QT += xml
CONFIG += staticlib
DEFINES += QT_UILIB_LIBRARY QT_DESIGNER

# Input
HEADERS += \
    resource.h \
    ui4.h \
    formbuilder.h \
    container.h \
    customwidget.h

SOURCES += \
    resource.cpp \
    ui4.cpp \
    formbuilder.cpp

include(../sharedcomponents.pri)
