# Project ID used by some IDEs
GUID 		= {8a42d838-be4f-4e2f-8f74-0051f19ee4f8}
TEMPLATE	= app
TARGET		= sound

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config
x11:QTDIR_build:REQUIRES	= nas

HEADERS		= sound.h
SOURCES		= sound.cpp
