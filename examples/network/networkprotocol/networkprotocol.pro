QTDIR_build:REQUIRES        = network full-config
TEMPLATE	= app
QCONFIG         += network
CONFIG		+= qt warn_on release
HEADERS		= nntp.h view.h
SOURCES		= main.cpp \
		  nntp.cpp view.cpp
TARGET		= networkprotocol
