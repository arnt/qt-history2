GUID 		= {9867e3b7-7e34-435c-83b2-5353287ed845}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
