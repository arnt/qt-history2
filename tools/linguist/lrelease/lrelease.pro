TEMPLATE        = app
CONFIG          += qt warn_on console
CONFIG          -= resource_fork
HEADERS         = ../shared/metatranslator.h \
                  ../shared/proparser.h
SOURCES         = main.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/proparser.cpp

QT += xml
include( ../../../src/qt_professional.pri )

TARGET          = lrelease
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
