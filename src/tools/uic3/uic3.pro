TEMPLATE = app
CONFIG += console qt_no_compat_warning
CONFIG -= resource_fork
build_all:CONFIG += release

QT += xml qt3support

DESTDIR = ../../../bin

include(../uic/uic.pri)

INCLUDEPATH += .

HEADERS += ui3reader.h \
           parser.h \
           domtool.h \
           widgetinfo.h \
           qt3to4.h

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
           qt3to4.cpp

DEFINES -= QT_COMPAT_WARNINGS
DEFINES += QT_COMPAT

target.path=$$[QT_INSTALL_BINS]
INSTALLS += target
