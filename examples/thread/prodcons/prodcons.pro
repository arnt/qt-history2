TEMPLATE	= app
CONFIG		+= qt warn_on
SOURCES		= prodcons.cpp
TARGET		= prodcons
CLEAN_FILES	= prodcons.out
QTDIR_build:REQUIRES	= thread
QTDIR_build:REQUIRES	= large-config
