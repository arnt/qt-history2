TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
HEADERS	= qdom.h qxml.h 
SOURCES	= qdom.cpp qxml.cpp 
TARGET		= qutil
DESTDIR		= $$QMAKE_LIBDIR_QT
VERSION		= 1.0.0

