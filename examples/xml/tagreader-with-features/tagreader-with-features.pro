# Project ID used by some IDEs
GUID 		= {91d6c338-8e87-45fa-9df8-15c4360b958c}
TEMPLATE	= app
TARGET          = tagreader-with-features

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= xml large-config

HEADERS		= structureparser.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
