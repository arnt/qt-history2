TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		=
SOURCES		= main.cpp
TARGET		= t3
QTDIR_build:REQUIRES="contains(QT_CONFIG, small-config)"
