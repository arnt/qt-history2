TEMPLATE = app
LANGUAGE = C++

CONFIG        += qt warn_on uic3

SOURCES        += finddialog.cpp \
        main.cpp \
        msgedit.cpp \
        phrase.cpp \
        phrasebookbox.cpp \
        phraselv.cpp \
        printout.cpp \
        simtexth.cpp \
        trwindow.cpp \
        listviews.cpp \
	../shared/metatranslator.cpp
HEADERS        += finddialog.h \
        msgedit.h \
        phrase.h \
        phrasebookbox.h \
        phraselv.h \
        printout.h \
        trwindow.h \
        listviews.h \
        ../shared/metatranslator.h


TRANSLATIONS        = linguist_de.ts \
                  linguist_fr.ts

DEFINES        += QT_KEYWORDS
QT += compat xml network
include( ../../../src/qt_professional.pri )

DESTDIR                = ../../../bin
TARGET                = linguist


LIBS        += -L$$QT_BUILD_TREE/lib -lqassistantclient

win32:RC_FILE        = linguist.rc

mac {
    staticlib:CONFIG -= global_init_link_order #yuck
    RC_FILE = linguist.icns
}

PROJECTNAME        = Qt Linguist

target.path=$$bins.path
INSTALLS        += target

linguisttranslations.files = *.qm
linguisttranslations.path = $$translations.path
INSTALLS += linguisttranslations

phrasebooks.path=$$data.path/phrasebooks
phrasebooks.files = ../phrasebooks/*
INSTALLS += phrasebooks
FORMS        = about.ui \
        statistics.ui
INCLUDEPATH        += ../shared ../../assistant/lib
RESOURCES += linguist.qrc
