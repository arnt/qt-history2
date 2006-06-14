TEMPLATE        = app
QT += xml
CONFIG          += qt warn_on console
CONFIG          -= app_bundle
DEFINES         *= PROPARSER_STORE_LINENUMBERS
build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}


HEADERS         = ../shared/metatranslator.h \
                  ../shared/translator.h \
                  ../shared/proparser.h \
                  ../shared/findsourcesvisitor.h \
                  ../shared/proparserutils.h \
                  ../shared/simtexth.h

SOURCES         = fetchtr.cpp \
                  main.cpp \
                  merge.cpp \
                  numberh.cpp \
                  sametexth.cpp \
                  ../shared/metatranslator.cpp \
                  ../shared/translator.cpp \
                  ../shared/proparser.cpp \
                  ../shared/findsourcesvisitor.cpp \
                  ../shared/simtexth.cpp

PROPARSERPATH = ../shared
INCLUDEPATH += $$PROPARSERPATH
# Input
HEADERS += $$PROPARSERPATH/proitems.h \
        $$PROPARSERPATH/abstractproitemvisitor.h \
        $$PROPARSERPATH/proreader.h
SOURCES += $$PROPARSERPATH/proitems.cpp \
        $$PROPARSERPATH/proreader.cpp


TARGET          = lupdate
INCLUDEPATH     += ../shared
DESTDIR          = $(QTDIR)/bin

target.path=$$[QT_INSTALL_BINS]
INSTALLS        += target
