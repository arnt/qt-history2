# Project ID used by some IDEs
GUID 		= {73790d4a-7ea4-400f-ae2c-e36298806b44}
TEMPLATE	= app
TARGET		= biff

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		= biff.h
SOURCES		= biff.cpp \
		  main.cpp
