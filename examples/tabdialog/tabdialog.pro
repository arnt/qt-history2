# Project ID used by some IDEs
GUID 		= {0210f57c-f92f-40df-9808-a8f312dd9713}
TEMPLATE	= app
TARGET		= tabdialog

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= tabdialog.h
SOURCES		= main.cpp \
		  tabdialog.cpp
