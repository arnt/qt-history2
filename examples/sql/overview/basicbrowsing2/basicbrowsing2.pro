# Project ID used by some IDEs
GUID 		= {fc560a4d-6980-4944-af92-4d53dc95c43c}
TEMPLATE	= app

QCONFIG += sql
CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp ../connection.cpp
