TEMPLATE        = app
CONFIG          += qt warn_on console
CONFIG          -= resource_fork
HEADERS         = ../shared/metatranslator.h
SOURCES         = main.cpp \
                  ../shared/metatranslator.cpp

QT += xml
include( ../../../src/qt_professional.pri )

TARGET          = qm2ts
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$bins.path
INSTALLS        += target
