TEMPLATE	= app
TARGET		= statistics

CONFIG		+= qt warn_on release
QT += compat
DEPENDPATH	= ../../include


HEADERS		= statistics.h
SOURCES		= statistics.cpp main.cpp
