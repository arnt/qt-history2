# Project ID used by some IDEs
GUID 		= {826ddfe6-a4a8-44da-89e4-7967766aa5f7}
TEMPLATE	= app
TARGET		= mdi

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = workspace full-config

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
