TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= tagreader.h
SOURCES		= tagreader.cpp \
                  structureparser.cpp
INTERFACES	=
TARGET          = tagreader
REQUIRES	= xml
