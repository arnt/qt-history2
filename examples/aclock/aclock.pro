GUID 		= {c95b6a36-ae6a-4c22-a2e3-959ebb091eb5}
TEMPLATE	= app
TARGET		= aclock

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= aclock.h
SOURCES		= aclock.cpp \
		  main.cpp
