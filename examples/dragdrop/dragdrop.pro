# Project ID used by some IDEs
GUID 		= {add7185f-fdfc-40b2-a96c-1ece6e62853d}
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
