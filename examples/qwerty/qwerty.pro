GUID 		= {3e957b43-c67a-4120-b59c-40957339dfb2}
TEMPLATE	= app
TARGET		= qwerty

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= qwerty.h
SOURCES		= main.cpp \
		  qwerty.cpp
