GUID 		= {36b2cd73-bb13-4b12-9a91-dae1db42eb7a}
TEMPLATE	= app
TARGET		= httpd

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network large-config

HEADERS		=
SOURCES		= httpd.cpp
