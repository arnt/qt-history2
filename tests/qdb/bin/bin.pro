TEMPLATE	= app
CONFIG		= xbase
DEFINES		+= XBASE_DEBUG
include(../include/shared.pri)
SOURCES	        += main.cpp
LIBPATH		+= ../lib
INCLUDEPATH	+= ../include
DESTDIR		= ../bin
TARGET          = qdb



