# Project ID used by some IDEs
GUID 		= {dbe2e44f-d3d9-4f63-87d7-d7dd3e9d2eba}
TEMPLATE	= app
TARGET		= splitter

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= medium-config

HEADERS		=
SOURCES		= splitter.cpp
