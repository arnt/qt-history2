TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= wizard.h
SOURCES		= main.cpp \
		  wizard.cpp
TARGET		= wizard
DEPENDPATH=../../include
QTDIR_build:REQUIRES=full-config
