TEMPLATE	= app
TARGET		= dragdrop

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= dropsite.h \
		  secret.h
SOURCES		= dropsite.cpp \
		  main.cpp \
		  secret.cpp
