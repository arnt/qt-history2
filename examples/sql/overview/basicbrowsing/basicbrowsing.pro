GUID 		= {2c067f3f-3184-4059-9427-696d20b8f440}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
