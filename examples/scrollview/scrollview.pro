# Project ID used by some IDEs
GUID 		= {aabfa176-c307-4445-80c8-18affd13c6e4}
TEMPLATE	= app
TARGET		= scrollview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= large-config

HEADERS		=
SOURCES		= scrollview.cpp
