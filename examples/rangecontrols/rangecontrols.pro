GUID 		= {a46c90aa-1945-4d5a-973b-acb29e2d061d}
TEMPLATE	= app
TARGET		= rangecontrols

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= rangecontrols.h
SOURCES		= main.cpp \
		  rangecontrols.cpp
