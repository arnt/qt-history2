TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= application.h
SOURCES		= application.cpp \
		  main.cpp
TARGET		= action
DEPENDPATH=../../include
REQUIRES=full-config
