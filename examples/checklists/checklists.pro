GUID 		= {4c707512-cda4-47b0-9d55-0c5af5746a62}
TEMPLATE	= app
TARGET		= checklists

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= checklists.h
SOURCES		= checklists.cpp \
		  main.cpp
