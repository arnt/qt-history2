TEMPLATE	= app
TARGET		= mdi

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = workspace full-config

HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
