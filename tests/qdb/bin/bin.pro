TEMPLATE	= app
include(../include/shared.pri)
SOURCES	        += main.cpp
LIBS		+= -lqdb
LIBPATH		+= ../lib
INCLUDEPATH	+= ../include
DESTDIR		= ../bin
TARGET          = qdb



