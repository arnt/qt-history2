TEMPLATE        = app
CONFIG          += qt warn_on console
CONFIG          -= app_bundle
HEADERS         = ../shared/metatranslator.h \
                  ../shared/proparser.h
SOURCES         = fetchtr.cpp \
                  main.cpp \
                  merge.cpp \
                  numberh.cpp \
                  sametexth.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/proparser.cpp

QT += xml
include( ../../../src/qt_professional.pri )

TARGET          = lupdate
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
