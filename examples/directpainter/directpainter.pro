# Project ID used by some IDEs
GUID 		= {f3bd7f18-8ee1-49b4-bbe7-4dc6f91c2e84}
TEMPLATE	= app

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = embedded small-config

HEADERS		=
SOURCES		= directpainter.cpp
TARGET		= directpainter
