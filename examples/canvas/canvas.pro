TEMPLATE	= app
TARGET		= canvas

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= canvas full-config

HEADERS		= canvas.h
SOURCES		= canvas.cpp main.cpp
