# Project ID used by some IDEs
GUID 		= {d11e0c68-e45d-4ae7-a44d-06a26f31e538}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
