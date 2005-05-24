TEMPLATE        = app
QT += xml
CONFIG          += qt warn_on console
CONFIG          -= app_bundle
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}
HEADERS         = ../shared/metatranslator.h \
                  ../shared/translator.h 
SOURCES         = main.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/translator.cpp

TARGET          = qm2ts
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
