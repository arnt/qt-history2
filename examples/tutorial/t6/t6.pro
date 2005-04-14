TEMPLATE	= app
CONFIG		+= qt warn_on
HEADERS		=
SOURCES		= main.cpp
TARGET		= t6
QTDIR_build:REQUIRES="contains(QT_CONFIG, large-config)"
