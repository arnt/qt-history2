GUID 		= {206c7ae9-f162-49ce-b0e7-f6a1c76b36a7}
TEMPLATE	= app
TARGET		= buttongroups

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= small-config

HEADERS		= buttongroups.h
SOURCES		= buttongroups.cpp \
		  main.cpp
