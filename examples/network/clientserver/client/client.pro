GUID 		= {04cbce85-68a6-49c2-abe1-7abf0dee9bc7}
TEMPLATE	= app
TARGET		= client

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config

HEADERS		=
SOURCES		= client.cpp
