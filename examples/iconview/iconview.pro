# Project ID used by some IDEs
GUID 		= {5d678075-7595-4193-8768-b8855c5fa3cb}
TEMPLATE	= app
TARGET		= iconview

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES        = iconview full-config

HEADERS		=
SOURCES		= main.cpp
