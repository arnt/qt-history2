# Project ID used by some IDEs
GUID		= {dd5ab041-202d-4856-a3be-763f8d2cc258}
TEMPLATE	= lib
TARGET		= qxt

CONFIG		+= qt staticlib warn_on release x11
DESTDIR		= ../../../lib
VERSION		= 0.3

HEADERS		= qxt.h
SOURCES		= qxt.cpp
DESTINCDIR	= ../../../include
