GUID 		= {016eab00-7567-403a-8697-a308ded2488a}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
