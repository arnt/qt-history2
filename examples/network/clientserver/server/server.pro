TEMPLATE	= app
TARGET		= server

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config

HEADERS		=
SOURCES		= server.cpp
