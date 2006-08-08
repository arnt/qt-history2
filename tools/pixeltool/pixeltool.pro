TEMPLATE = app
CONFIG  += qt warn_on
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

DESTDIR     = ../../bin

DEPENDPATH += .
INCLUDEPATH += .
TARGET = pixeltool

# Input
SOURCES += main.cpp qpixeltool.cpp
HEADERS += qpixeltool.h

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
