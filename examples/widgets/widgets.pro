# Project ID used by some IDEs
GUID 		= {4570bb58-e691-4a86-9598-be542ca25d4b}
TEMPLATE	= app
TARGET		= widgets

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include
INCLUDEPATH	+= ../aclock ../dclock

QTDIR_build:REQUIRES	= full-config

HEADERS		= widgets.h ../aclock/aclock.h ../dclock/dclock.h
SOURCES		= main.cpp widgets.cpp ../aclock/aclock.cpp ../dclock/dclock.cpp
