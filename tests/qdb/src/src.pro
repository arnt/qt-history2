TEMPLATE	= lib
CONFIG		= qt warn_on debug
LIBS		= -lxdb
SOURCES	        = sqlinterpreter.cpp environment.cpp filedriver_xbase.cpp
HEADERS		= ../include/sqlinterpreter.h ../include/environment.h
INCLUDEPATH	+= ../include
TARGET          = qdb
VERSION		= 1.0.0
DESTDIR		= ../lib

