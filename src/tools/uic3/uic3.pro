HEADERS += ui3reader.h \
           parser.h \
           domtool.h \
           globaldefs.h \
           widgetinfo.h \
           ../uic/ui4.h \
           ../uic/blockingprocess.h
SOURCES += main.cpp \
           ui3reader.cpp \
           parser.cpp \
           domtool.cpp \
           object.cpp \
           subclassing.cpp \
           form.cpp \
           converter.cpp \
           widgetinfo.cpp \
           embed.cpp \
           ../uic/blockingprocess.cpp

CONFIG += console
CONFIG -= resource_fork
QT += xml compat
TEMPLATE = app
INCLUDEPATH += . ../uic

DESTDIR = ../../../bin
