TEMPLATE        = app
QT += xml
CONFIG          += qt warn_on console
CONFIG          -= app_bundle
build_all:CONFIG += release
HEADERS         = ../shared/metatranslator.h
SOURCES         = main.cpp \
                  ../shared/metatranslator.cpp

TARGET          = qm2ts
INCLUDEPATH     += ../shared
DESTDIR         = ../../../bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
