GUID 		= {5da202de-3f68-490b-bbb7-a50c4f15a92b}
TEMPLATE	= app
TARGET		= listboxcombo

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= large-config

HEADERS		= listboxcombo.h
SOURCES		= listboxcombo.cpp \
		  main.cpp
