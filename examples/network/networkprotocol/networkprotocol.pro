GUID 		= {58fb6263-538a-4723-822b-d6af93dd809b}
TEMPLATE	= app
TARGET		= networkprotocol

QCONFIG         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network full-config

HEADERS		= nntp.h view.h
SOURCES		= main.cpp \
		  nntp.cpp view.cpp
