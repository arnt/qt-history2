QTDIR_build:REQUIRES        = network full-config
TEMPLATE	= app
QCONFIG         += network
CONFIG		+= qt warn_on release
HEADERS		= 
SOURCES		= server.cpp
TARGET		= server
