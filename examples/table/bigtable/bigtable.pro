# Project ID used by some IDEs
GUID 		= {f2f4a41c-7602-4fb3-b62c-f61f9856a8fe}
TEMPLATE	= app
TARGET		= bigtable

CONFIG		+= qt warn_on release
DEPENDPATH 	= ../../include

QTDIR_build:REQUIRES 	= table full-config

HEADERS		=
SOURCES		= main.cpp
