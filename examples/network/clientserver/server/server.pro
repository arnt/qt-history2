# Project ID used by some IDEs
GUID 		= {85ee07e0-a884-49ec-b5f8-d271c737ce63}
TEMPLATE	= app
TARGET		= server

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config

HEADERS		=
SOURCES		= server.cpp
