# Project ID used by some IDEs
GUID 		= {b8ffca6e-9331-4bb7-bdff-874aefb49972}
TEMPLATE	= app
TARGET		= tux

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= small-config

HEADERS		=
SOURCES		= tux.cpp
INTERFACES	=
