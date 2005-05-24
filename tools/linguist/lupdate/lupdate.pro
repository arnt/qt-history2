TEMPLATE        = app
QT += xml
CONFIG          += qt warn_on console
CONFIG          -= app_bundle
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
HEADERS         = ../shared/metatranslator.h \
                  ../shared/translator.h \
                  ../shared/proparser.h
SOURCES         = fetchtr.cpp \
                  main.cpp \
                  merge.cpp \
                  numberh.cpp \
                  sametexth.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/translator.cpp \
                  ../shared/proparser.cpp

TARGET          = lupdate
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
