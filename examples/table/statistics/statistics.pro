TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS	= statistics.h
SOURCES	= statistics.cpp main.cpp
TARGET		= statistics
DEPENDPATH	= ../../include
QTDIR_build:REQUIRES=table full-config
