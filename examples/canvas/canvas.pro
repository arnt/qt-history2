GUID 		= {26e3cba0-2c6d-45f0-8db5-e933da3d53ba}
TEMPLATE	= app
TARGET		= canvas

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= canvas full-config

HEADERS		= canvas.h
SOURCES		= canvas.cpp main.cpp
