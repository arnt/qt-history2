GUID 		= {92f481df-56e8-4305-b1e2-19e613718934}
TEMPLATE	= app
TARGET		= qmag

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= qmag.cpp
