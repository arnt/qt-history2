# Project ID used by some IDEs
GUID 		= {c56b7af9-ae4a-42f7-824f-534914caaff7}
TEMPLATE	= app
TARGET		= wizard

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= wizard.h
SOURCES		= main.cpp \
		  wizard.cpp
