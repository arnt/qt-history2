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
		  ../shared/metatranslator.cpp
FORMS		= about.ui

IMAGES		+= images/accel.xpm \
		   images/book.png \
		   images/check_danger.xpm \
		   images/check_obs.xpm \
		   images/check_off.xpm \
		   images/check_on.xpm \
		   images/d_book.png \
		   images/d_doneandnext.png \
		   images/d_editcopy.png \
		   images/d_editcut.png \
		   images/d_editpaste.png \
		   images/d_fileopen.png \
		   images/d_filesave.png \
		   images/d_next.png \
		   images/d_nextunfinished.png \
		   images/d_prev.png \
		   images/d_prevunfinished.png \
		   images/d_print.png \
		   images/d_redo.png \
		   images/d_searchfind.png \
		   images/d_undo.png \
		   images/doneandnext.png \
		   images/editcopy.png \
		   images/editcut.png \
		   images/editpaste.png \
		   images/endpunct.xpm \
		   images/fileopen.png \
		   images/filesave.png \
		   images/icon.png \
		   images/next.png \
		   images/nextunfinished.png \
		   images/pagecurl.png \
		   images/phrase.xpm \
		   images/prev.png \
		   images/prevunfinished.png \
		   images/print.png \
		   images/redo.png \
		   images/searchfind.png \
		   images/splash.png \
		   images/undo.png \
		   images/whatsthis.xpm

TRANSLATIONS	= linguist_de.ts \
		  linguist_fr.ts

DEFINES 	+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

DESTDIR		= ../../../bin
TARGET		= linguist
INCLUDEPATH	+= ../pics \
		  ../shared

unix:LIBS	+= -L$$QT_BUILD_TREE/lib -lqassistantclient
win32 {
    LIBS	+= $$QT_BUILD_TREE/lib/qassistantclient.lib
    RC_FILE	= linguist.rc
}
mac {
    staticlib:CONFIG -= global_init_link_order #yuck
    RC_FILE = linguist.icns
}

PROJECTNAME	= Qt Linguist

target.path=$$bins.path
INSTALLS	+= target

phrasebooks.path=$$data.path/phrasebooks
phrasebooks.files = ../phrasebooks/*
INSTALLS += phrasebooks
