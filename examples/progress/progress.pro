# Project ID used by some IDEs
GUID 		= {4aa9b436-03df-44c2-90c9-e570dd2b8b18}
TEMPLATE	= app
TARGET		= progress

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= progress.cpp
