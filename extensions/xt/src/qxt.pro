TEMPLATE	= lib
TARGET		= qxt

CONFIG		+= qt staticlib warn_on release x11
DESTDIR		= ../../../lib
VERSION		= 0.3

HEADERS		= qxt.h
SOURCES		= qxt.cpp
DESTINCDIR	= ../../../include/QXT
