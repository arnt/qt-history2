TEMPLATE	= app
TARGET		= networkprotocol

QT         += network
CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES        = network "contains(QT_CONFIG, full-config)"

HEADERS		= nntp.h view.h
SOURCES		= main.cpp \
		  nntp.cpp view.cpp
