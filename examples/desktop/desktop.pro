GUID 		= {c838a818-0824-4738-a8c9-40e5317af2e1}
TEMPLATE	= app
TARGET		= desktop

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= desktop.cpp
