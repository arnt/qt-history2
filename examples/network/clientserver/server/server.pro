TEMPLATE	= app
TARGET		= server

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= server.cpp
