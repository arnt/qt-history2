# Project ID used by some IDEs
GUID 		= {19648f4a-51e2-4b85-9750-980293e876f0}
TEMPLATE	= app
TARGET		= layout

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		=
SOURCES		= layout.cpp
