# Project ID used by some IDEs
GUID 		= {1404492c-79c4-4119-9a98-b7a5abb00c36}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
