# Project ID used by some IDEs
GUID 		= {f3b8978b-20dd-4e86-8338-8a2a155c1801}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
