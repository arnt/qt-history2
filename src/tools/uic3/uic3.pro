TEMPLATE = app
CONFIG += console
CONFIG -= resource_fork

QT += xml compat

mac:QT += network sql

DESTDIR = ../../../bin

include(../uic/uic.pri)

INCLUDEPATH += .

HEADERS += ui3reader.h \
           parser.h \
           domtool.h \
           widgetinfo.h

SOURCES += main.cpp \
           ui3reader.cpp \
           parser.cpp \
           domtool.cpp \
           object.cpp \
           subclassing.cpp \
           form.cpp \
           converter.cpp \
           widgetinfo.cpp \
           embed.cpp

