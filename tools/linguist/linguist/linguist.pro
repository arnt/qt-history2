TEMPLATE = app
LANGUAGE = C++

CONFIG        += qt warn_on assistant

SOURCES        += finddialog.cpp \
	about.cpp \
	statistics.cpp \
	contextmodel.cpp \
	messagemodel.cpp \
	phrasemodel.cpp \
	msgedit.cpp \
        main.cpp \
        phrase.cpp \
        phrasebookbox.cpp \
        printout.cpp \
        simtexth.cpp \
        trwindow.cpp \
	../shared/metatranslator.cpp
HEADERS        += finddialog.h \
	about.h \
	statistics.h \
	contextmodel.h \
	messagemodel.h \
	phrasemodel.h \
	msgedit.h \
        phrase.h \
        phrasebookbox.h \
        printout.h \
        simtexth.h \
        trwindow.h \
        ../shared/metatranslator.h

DEFINES        += QT_KEYWORDS
QT += xml network
include( ../../../src/qt_professional.pri )

DESTDIR                = ../../../bin
TARGET                = linguist

win32:RC_FILE        = linguist.rc

mac {
    staticlib:CONFIG -= global_init_link_order #yuck
    ICON = linguist.icns
    TARGET = Linguist
}

PROJECTNAME        = Qt Linguist

target.path=$$bins.path
INSTALLS        += target

linguisttranslations.files = *.qm
linguisttranslations.path = $$translations.path
INSTALLS += linguisttranslations

phrasebooks.path=$$data.path/phrasebooks
phrasebooks.files = $$QT_SOURCE_TREE/tools/linguist/phrasebooks/*
INSTALLS += phrasebooks
FORMS     = about.ui \
            statistics.ui \
            phrasebookbox.ui \
            finddialog.ui
INCLUDEPATH        += ../shared ../../assistant/lib
RESOURCES += linguist.qrc
