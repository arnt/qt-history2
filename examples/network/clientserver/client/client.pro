TEMPLATE	= app
TARGET		= client

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network "contains(QT_CONFIG, full-config)"

HEADERS		=
SOURCES		= client.cpp
