# Project ID used by some IDEs
GUID 		= {304b500f-3bd1-4c4c-81cb-fd3ead8b96da}
TEMPLATE	= app
TARGET          = blob

CONFIG		+= qt warn_on release
win32:CONFIG	+= console

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp
INTERFACES	=
