GUID 		= {a2a6bc23-0364-4b10-a755-5636a3bb1e44}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
