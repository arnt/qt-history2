TEMPLATE	= app
TARGET		= httpd

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network "contains(QT_CONFIG, large-config)"

HEADERS		=
SOURCES		= httpd.cpp
