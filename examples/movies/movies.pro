GUID 		= {cf6c7d5b-74b4-4e4b-b5c7-7d1e0f382457}
TEMPLATE	= app
TARGET		= movies

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= main.cpp
