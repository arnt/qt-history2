GUID 		= {c9a33c12-d7b2-470d-b451-355b1f9f87a9}
TEMPLATE	= app
TARGET		= picture

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= picture.cpp
