GUID 		= {591c1681-a844-4145-81f6-6a04ac75f048}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
