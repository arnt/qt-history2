GUID 		= {1bc38738-61bb-48ca-907c-6e765159ee45}
TEMPLATE	= app
TARGET		= listbox

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		= listbox.h
SOURCES		= listbox.cpp \
		  main.cpp
