# Project ID used by some IDEs
GUID 		= {61388ca7-fc7a-42be-8d36-e5e0c5383d29}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
