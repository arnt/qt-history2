# Project ID used by some IDEs
GUID 		= {f3bce62c-6f9c-4cf9-b6e5-4db9a14c140f}
TEMPLATE	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
