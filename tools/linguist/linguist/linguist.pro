TEMPLATE	= app
CONFIG		= qt warn_on release 
HEADERS		= finddialog.h \
		  msgedit.h \
		  phrase.h \
		  phrasebookbox.h \
		  phraselv.h \
		  printout.h \
		  trwindow.h \
		  listviews.h \
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
		  ../shared/metatranslator.cpp
INCLUDEPATH	= ../pics \
		  ../shared
DESTDIR	= $(QTDIR)/bin
unix:LIBS += -lpthread
INCLUDEPATH	+= $(QTDIR)/src/kernel
