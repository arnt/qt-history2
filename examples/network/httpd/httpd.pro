QTDIR_build:REQUIRES        = network large-config
TEMPLATE	= app
QCONFIG         += network
CONFIG		+= qt warn_on release
HEADERS		= 
SOURCES		= httpd.cpp
TARGET		= httpd
