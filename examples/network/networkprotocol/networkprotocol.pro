TEMPLATE	= app
TARGET		= networkprotocol

QT         += network compat
CONFIG		+= qt warn_on release

HEADERS		= nntp.h view.h
SOURCES		= main.cpp \
		  nntp.cpp view.cpp
