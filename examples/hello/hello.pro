GUID 		= {748346a8-bcf3-4691-a4ce-58f04e413b1d}
TEMPLATE	= app
TARGET		= hello

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES=

HEADERS		= hello.h
SOURCES		= hello.cpp \
		  main.cpp
