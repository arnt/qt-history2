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
		  ../shared/metatranslator.cpp
FORMS		= about.ui

DEFINES 	+= QT_INTERNAL_XML
include( ../../../src/qt_professional.pri )

TARGET		= linguist
INCLUDEPATH	= ../pics \
		  ../shared \
		  ../../../src/kernel
DESTDIR		= ../../../bin

target.path=$$QT_INSTALL_BINPATH
isEmpty(target.path):target.path=$$QT_PREFIX/bin
INSTALLS	+= target
