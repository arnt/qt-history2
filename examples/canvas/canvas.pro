TEMPLATE	= app
TARGET		= canvas

CONFIG		+= qt warn_on release
QT         += canvas compat
DEPENDPATH	= ../../include

HEADERS		= canvas.h
SOURCES		= canvas.cpp main.cpp
