# Project ID used by some IDEs
GUID 		= {4fedf5f4-59aa-41ef-a4e0-1c252d86a37a}
TEMPLATE	= app
TARGET		= fontdemo

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES 	= full-config

HEADERS		= viewer.h
SOURCES		= simple-qfont-demo.cpp \
	          viewer.cpp
