# Project ID used by some IDEs
GUID 		= {d2bf055f-8d28-48a5-8376-69805ad485a6}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

REQUIRES 	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp

