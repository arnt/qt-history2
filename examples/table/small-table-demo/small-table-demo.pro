# Project ID used by some IDEs
GUID 		= {4ba67b65-d82f-4694-b724-4ae46d6c09d4}
TEMPLATE	= app
TARGET		= smalltable

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES 	= table full-config

HEADERS		=
SOURCES		= main.cpp
