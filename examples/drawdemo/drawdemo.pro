# Project ID used by some IDEs
GUID 		= {9d75537e-e5cd-42c8-83be-4f75ae1f87fd}
TEMPLATE	= app
TARGET		= drawdemo

CONFIG		+= qt warn_on release
DEPENDPATH	= ../../include

QTDIR_build:REQUIRES	= full-config

HEADERS		=
SOURCES		= drawdemo.cpp
