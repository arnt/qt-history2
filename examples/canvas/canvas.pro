TEMPLATE	= app
TARGET		= canvas

CONFIG		+= qt warn_on release
QT         += canvas compat
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= canvas full-config

HEADERS		= canvas.h
SOURCES		= canvas.cpp main.cpp
