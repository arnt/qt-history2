# Project ID used by some IDEs
GUID 		= {b69cc9ea-c26d-48f3-b37f-ff0a449e481b}
TEMPLATE	= app
TARGET		= action

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
