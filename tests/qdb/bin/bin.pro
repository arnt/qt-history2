TEMPLATE	= app
include(../include/shared.pri)
SOURCES	        += main.cpp
DEFINES		+= HAVE_CONFIG_H
LIBS		+= -lqdb
LIBPATH		+= ../lib
INCLUDEPATH	+= ../include
DESTDIR		= ../bin
TARGET          = qdb



