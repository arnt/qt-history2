TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
HEADERS	= ../../../include/qdom.h \
		  ../../../include/qxml.h \
		  ../../../include/qiconview.h
SOURCES	= ../../../src/xml/qdom.cpp \
		   ../../../src/xml/qxml.cpp \
		   ../../../src/iconview/qiconview.cpp
TARGET		= qutil
DESTDIR		= $(QTDIR)/lib
VERSION		= 1.0.0

