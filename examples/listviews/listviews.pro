GUID 		= {97b4534d-598b-49bf-a287-584f14d8ddfd}
TEMPLATE	= app
TARGET		= listviews

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= listviews.h
SOURCES		= listviews.cpp \
		  main.cpp
