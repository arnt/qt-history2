# Project ID used by some IDEs
GUID 		= {3ec0370e-b2db-47fa-80f4-29800b63de67}
TEMPLATE	= app
TARGET		= tooltip

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= tooltip.h
SOURCES		= main.cpp \
		  tooltip.cpp
