GUID 	 = {4604aee3-4456-42de-9dfa-fc5228b166a9}
TEMPLATE = lib
TARGET	 = qnp

CONFIG  -= dll
CONFIG  += qt x11 release staticlib
DESTDIR	 = ../../../lib
VERSION	 = 0.4

SOURCES		= qnp.cpp
unix:HEADERS   += qnp.h
win32:HEADERS	= ../../../include/qnp.h
win32:LIBS     += -lqtmain
MOC_DIR		= .
DESTINCDIR	= ../../../include
