# Project ID used by some IDEs
GUID 		= {4c8dca40-bf78-490e-8334-ef8cf8bbe7e4}
TEMPLATE	= app
TARGET		= dirview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= dirview.h
SOURCES		= dirview.cpp \
		  main.cpp
