# Project ID used by some IDEs
GUID 		= {be5b7ad8-c8b7-4776-b7f9-2cfdb9b22402}
TEMPLATE	= app
TARGET		= semaphores

CONFIG		+= qt warn_on release thread

QTDIR_build:REQUIRES        = thread full-config

HEADERS		=
SOURCES		= main.cpp
INTERFACES	=
