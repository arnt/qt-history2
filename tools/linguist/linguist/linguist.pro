TEMPLATE	= app
CONFIG		+= qt warn_on
HEADERS		= finddialog.h \
		  msgedit.h \
		  phrase.h \
		  phrasebookbox.h \
		  phraselv.h \
		  printout.h \
		  trwindow.h \
		  listviews.h \
		  splashloader.h \
		  logoloader.h \
		  ../pics/images.h \
		  ../shared/metatranslator.h
SOURCES		= finddialog.cpp \
		  main.cpp \
		  msgedit.cpp \
		  phrase.cpp \
		  phrasebookbox.cpp \
		  phraselv.cpp \
		  printout.cpp \
		  simtexth.cpp \
		  trwindow.cpp \
		  listviews.cpp \
		  splashloader.cpp \
		  logoloader.cpp \
		  ../shared/metatranslator.cpp
FORMS		= about.ui

win32:RC_FILE	= linguist.rc
TRANSLATIONS	= linguist_de.ts \
		  linguist_fr.ts

DEFINES 	+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

DESTDIR		= ../../../bin
TARGET		= linguist
INCLUDEPATH	+= ../pics \
		  ../shared

win32:LIBS	+= $$QT_BUILD_TREE/lib/qassistantclient.lib
unix {
	LIBS	+= -L$$QT_BUILD_TREE/lib -lqassistantclient
}

PROJECTNAME	= Qt Linguist

target.path=$$bins.path
INSTALLS	+= target

phrasebooks.path=$$data.path/phrasebooks
phrasebooks.files = ../phrasebooks/*
INSTALLS += phrasebooks
