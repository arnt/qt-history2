TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
HEADERS	= ../../../include/qdom.h \
		  ../../../include/qxml.h \
		  qprocess.h \
		  ../../../include/qiconview.h
SOURCES	= ../../../src/xml/qdom.cpp \
		   ../../../src/xml/qxml.cpp \
		   qprocess.cpp \
		   ../../../src/iconview/qiconview.cpp
unix:SOURCES	+= qprocess_unix.cpp
win32:SOURCES	+= qprocess_win.cpp
TARGET		= qutil
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0

