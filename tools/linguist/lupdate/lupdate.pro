TEMPLATE	= app
CONFIG		= qt warn_on release
HEADERS		= ../shared/metatranslator.h
SOURCES		= fetchtr.cpp \
		  main.cpp \
		  merge.cpp \
		  numberh.cpp \
		  sametexth.cpp \
		  ../shared/metatranslator.cpp
INCLUDEPATH	= ../shared
DESTDIR	= $(QTDIR)/bin
