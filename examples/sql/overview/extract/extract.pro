# Project ID used by some IDEs
GUID 	 	= {0802e9d4-7a5a-4451-959e-cb39cc169847}
TEMPLATE 	= app

QCONFIG		+= sql
CONFIG		+= qt warn_on release
DEPENDPATH	=../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
