TEMPLATE	= app
include(../include/shared.pri)
SOURCES	        += qdb.cpp
LIBS		+= -lqdb
LIBPATH		+= ../lib
INCLUDEPATH	+= ../include
DESTDIR		= ../bin
TARGET          = qdb



