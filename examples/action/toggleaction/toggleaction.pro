# Project ID used by some IDEs
GUID 		= {611933fa-91cf-4aa3-809f-f80b3c095b4c}
TEMPLATE	= app
TARGET		= toggleaction

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= toggleaction.cpp
